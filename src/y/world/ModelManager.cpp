/*
 * ModelManager.cpp
 *
 *  Created on: 19.01.2020
 *      Author: michi
 */

#include "ModelManager.h"
#include "Model.h"
#include "components/Collider.h"
#include "components/Animator.h"
#include "components/SolidBody.h"
#include "components/Skeleton.h"
#include "../y/Entity.h"
#include "../y/EngineData.h"
#include <lib/math/complex.h>
#include <lib/kaba/kaba.h>
#include <lib/config.h>
#include "../helper/ResourceManager.h"
#if __has_include("../plugins/PluginManager.h")
#include "../plugins/PluginManager.h"
#endif
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <lib/doc/chunked.h>
#include <lib/ygraphics/graphics-impl.h>
#include "../meta.h"
#include <lib/yrenderer/MaterialManager.h>


namespace yrenderer {
	ygfx::Alpha parse_alpha_i(int a); // Material.h
}


ModelTemplate::ModelTemplate(Model *m) {
	model = m;
	solid_body = nullptr;
	mesh_collider = nullptr;
	animator = nullptr;
	skeleton = nullptr;
}

yrenderer::MaterialManager *chunked_file_parser_get_material_manager(ChunkedFileParser *p);
ResourceManager *chunked_file_parser_get_resource_manager(ChunkedFileParser *p);


Stream *load_file_x(const Path &filename, int &version) {

	auto f = os::fs::open(filename, "rb");
	char c = f->read_char();
	if (c == 'b') {
		version = f->read_word();
		return f;
	} else if (c == 't') {
		/*delete f;
		f = os::fs::open(filename, "rt");
		f->read_char();
		version = f->read_word();
		return f;*/
		throw Exception("text format not supported anymore");
	} else {
		throw Exception("File format unreadable!");
	}
	return nullptr;
}


color file_read_color4i(Stream *f) {
	int a = f->read_int();
	int r = f->read_int();
	int g = f->read_int();
	int b = f->read_int();
	return color((float)a/255.0f, (float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}

ivec4 read_ivec4(Stream *f) {
	ivec4 v;
	v.i = f->read_int();
	v.j = f->read_int();
	v.k = f->read_int();
	v.l = f->read_int();
	return v;
}

vec4 read_vec4(Stream *f) {
	vec4 v;
	v.x = f->read_float();
	v.y = f->read_float();
	v.z = f->read_float();
	v.w = f->read_float();
	return v;
}

void write_ivec4(Stream *f, const ivec4 &v) {
	f->write_int(v.i);
	f->write_int(v.j);
	f->write_int(v.k);
	f->write_int(v.l);
}

void write_vec4(Stream *f, const vec4 &v) {
	f->write_float(v.x);
	f->write_float(v.y);
	f->write_float(v.z);
	f->write_float(v.w);
}



vec3 get_normal_by_index(int index) {
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vec3( cos(wxy) * swz, sin(wxy) * swz, cwz);
}



//--------------------------------------------------------------------------------------------------
// hopefully these functions will be obsolete with the next fileformat

// how big is the model
void AppraiseDimensions(Model *m) {
	float rad = 0;

	// bounding box (visual mesh[0])
	m->prop.min = m->prop.max = v_0;
	for (int i=0;i<m->mesh[0]->vertex.num;i++) {
		m->prop.min._min(m->mesh[0]->vertex[i]);
		m->prop.max._max(m->mesh[0]->vertex[i]);
		float r = _vec_length_fuzzy_(m->mesh[0]->vertex[i]);
		if (r > rad)
			rad = r;
	}

	// physical skin
	auto col = m->_template->mesh_collider;
	if (col) {
		for (int i=0;i<col->phys->vertex.num;i++) {
			float r = _vec_length_fuzzy_(col->phys->vertex[i]);
			if (r > rad)
				rad = r;
		}
		for (auto &b: col->phys->balls) {
			float r = _vec_length_fuzzy_(col->phys->vertex[b.index]) + b.radius;
			if (r > rad)
				rad = r;
		}
	}
	m->prop.radius = rad;
}


void PostProcessPhys(Model *m, PhysicalMesh *s) {
	auto col = m->_template->mesh_collider;
	if (col) {
		col->phys_absolute.p.clear();
		col->phys_absolute.pl.clear();
	}
	m->_ResetPhysAbsolute_();
}


namespace modelmanager {

class ChunkMeta : public FileChunk<Model, Model> {
public:
	ChunkMeta() : FileChunk("meta") {}
	void create() override {
		me = parent;
	}
	void read(Stream *f) override {
		[[maybe_unused]] int version = f->read_int();
		auto sb = me->_template->solid_body;

		f->read_vector(&me->prop.min);
		f->read_vector(&me->prop.max);
		me->prop.radius = f->read_float();

		// physics
		sb->mass = f->read_float();
		for (int i=0;i<9;i++)
			sb->theta_0.e[i] = f->read_float();
		sb->active = f->read_bool();
		sb->passive = f->read_bool();
	}
	void write(Stream *f) override {}
};

class ChunkOldMeta : public FileChunk<Model, Model> {
public:
	ChunkOldMeta() : FileChunk("xxx1") {}
	void create() override {
		me = parent;
	}
	void read(Stream *f) override {
		// Object Data
		me->script_data.name = f->read_str();
		/*me->script_data.description =*/ f->read_str();

		// Inventary
		int n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_str();
	}
	void write(Stream *f) override {}
};

class ChunkMaterial : public FileChunk<Model, yrenderer::Material> {
public:
	ChunkMaterial() : FileChunk("material") {}
	void create() override {
		//me = new Material(chunked_file_parser_get_resource_manager(root));
		//parent->material.add(me);
	}
	void read(Stream *f) override {
		me = chunked_file_parser_get_material_manager(root)->load(f->read_str());
		parent->material.add(me);
		bool user_colors = f->read_bool();
		if (user_colors) {
			me->albedo = file_read_color4i(f);
			me->emission = file_read_color4i(f);
			me->metal = f->read_float();
			me->roughness = f->read_float();
		} else {
			file_read_color4i(f);
			file_read_color4i(f);
			f->read_float();
			f->read_float();
		}

		auto alpha_mode = (yrenderer::TransparencyMode)f->read_int();
		if (alpha_mode != yrenderer::TransparencyMode::DEFAULT) {
			me->pass0.mode = alpha_mode;
			me->pass0.source = yrenderer::parse_alpha_i(f->read_int());
			me->pass0.destination = yrenderer::parse_alpha_i(f->read_int());
			me->pass0.factor = f->read_float();
			me->pass0.z_buffer = f->read_bool();
		} else {
			f->read_int();
			f->read_int();
			f->read_float();
			f->read_bool();
		}
		int nt = f->read_int();
		parent->num_uvs.add(nt);
		if (nt > me->textures.num)
			me->textures.resize(nt);
		for (int t=0;t<nt;t++) {
			Path fn = f->read_str();
			me->textures[t] = chunked_file_parser_get_resource_manager(root)->load_texture(fn);
		}
	}
	void write(Stream *f) override {}
};

static int _model_parser_tria_mesh_count;

class ChunkMesh : public FileChunk<Model, Mesh> {
public:
	ChunkMesh() : FileChunk("mesh") {}
	void create() override {
		me = new Mesh;
		me->owner = parent;
		parent->mesh[_model_parser_tria_mesh_count ++] = me;
	}
	void read(Stream *f) override {
		[[maybe_unused]] int version = f->read_int();
		int flags = f->read_int();

		// vertices
		int nv = f->read_int();
		me->vertex.resize(nv);
		me->bone_index.resize(nv);
		me->bone_weight.resize(nv);
		for (int j=0; j<nv; j++)
			f->read_vector(&me->vertex[j]);
		if (flags & 0x1)
			for (int j=0; j<nv; j++) {
				me->bone_index[j] = read_ivec4(f);
				me->bone_weight[j] = read_vec4(f);
			}

		// skin vertices
		Array<complex> skin_vert;
		int nsv = f->read_int();
		skin_vert.resize(nsv);
		for (int j=0;j<skin_vert.num;j++) {
			skin_vert[j].x = f->read_float();
			skin_vert[j].y = f->read_float();
		}

		// sub skins
		me->sub.resize(parent->material.num);
		for (int m=0;m<parent->material.num;m++) {
			auto *sub = &me->sub[m];
			int num_uvs = parent->num_uvs[m];
			// triangles
			sub->num_triangles = f->read_int();
			sub->triangle_index.resize(sub->num_triangles * 3);
			sub->skin_vertex.resize(num_uvs * sub->num_triangles * 6);
			sub->normal.resize(sub->num_triangles * 3);
			// vertices
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->triangle_index[i] = f->read_int();
			// skin vertices
			//msg_write(format("%d  %d", parent->material[m]->textures.num, sub->num_triangles));
			for (int i=0;i<num_uvs * sub->num_triangles * 3;i++){
				int sv = f->read_int();
				sub->skin_vertex[i * 2    ] = skin_vert[sv].x;
				sub->skin_vertex[i * 2 + 1] = skin_vert[sv].y;
			}
			// normals
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->normal[i] = get_normal_by_index(f->read_word());

			sub->force_update = true;
			sub->vertex_buffer = nullptr;
		}
	}
	void write(Stream *f) override {}
};

class ChunkPhysicalMesh : public FileChunk<Model, PhysicalMesh> {
public:
	ChunkPhysicalMesh() : FileChunk("physmesh") {}
	void define_children() override {
	}
	void create() override {
		me = new PhysicalMesh;
		parent->_template->mesh_collider->phys = me;
	}
	void read(Stream *f) override {
		[[maybe_unused]] int version = f->read_int();

		// vertices
		int num_vert = f->read_int();
		me->vertex.resize(num_vert);
		me->bone_nr.resize(num_vert);
		for (int j=0;j<num_vert;j++)
			f->read_vector(&me->vertex[j]);
		for (int j=0;j<num_vert;j++)
			me->bone_nr[j] = f->read_int();

		// triangles
		f->read_int();

		// balls
		int num_balls = f->read_int();
		me->balls.resize(num_balls);
		for (auto &b: me->balls) {
			b.index = f->read_int();
			b.radius = f->read_float();
		}

		// polyhedra
		int num_poly = f->read_int();
		me->poly.resize(num_poly);
		for (int i=0;i<num_poly;i++){
			ConvexPolyhedron *p = &me->poly[i];
			p->num_faces = f->read_int();
			for (int j=0;j<p->num_faces;j++) {
				p->face[j].num_vertices = f->read_int();
				for (int k=0;k<p->face[j].num_vertices;k++)
					p->face[j].index[k] = f->read_int();
			}
		}

		// cylinders
		int num_cyl = f->read_int();
		for (int i=0; i<num_cyl; i++) {
			Cylinder c;
			c.index[0] = f->read_int();
			c.index[1] = f->read_int();
			c.radius = f->read_float();
			c.round = f->read_bool();
			me->cylinders.add(c);
		}
	}
	void write(Stream *f) override {}
};


class ChunkSkeleton : public FileChunk<Model, Model> {
public:
	ChunkSkeleton() : FileChunk("skeleton") {}
	void create() override {
		me = parent;
	}
	void read(Stream *f) override {
		[[maybe_unused]] int version = f->read_int();
		auto sk = me->_template->skeleton;
		int n = f->read_int();
		sk->bones.resize(n);
		sk->parents.resize(n);
		sk->dpos.resize(n);
		sk->pos0.resize(n);
		sk->filename.resize(n);
		foreachi ([[maybe_unused]] auto &b, sk->bones, i) {
			f->read_vector(&sk->dpos[i]);
			sk->parents[i] = f->read_int();
			sk->filename[i] = f->read_str();
			//b.model = ModelManager::load(filename);
		}
		f->read_str();
	}
	void write(Stream *f) override {}
};

#if 0
void import_animations(DataModel *me, const Array<ModelFrame> &frames_vert, const Array<ModelFrame> &frames_skel, const Array<int> &offsets) {
	//msg_write(ia2s(offsets));
	foreachi (auto &m, me->move, mi) {
		if (m.type == MOVE_TYPE_VERTEX)
			m.frame = frames_vert.sub(offsets[mi], m.frame.num);
		else if (m.type == MOVE_TYPE_SKELETAL) {
			//msg_write(format("sk %d  %d    %d", offsets[mi], m.frame.num, frames_skel.num));
			for (int i=0; i<m.frame.num; i++)
				m.frame[i] = frames_skel[offsets[mi] + i];
			//m.frame = frames_skel.sub(offsets[mi], m.frame.num);
		}
	}
}
#endif


// TODO later write skeleton animations into separate file...
// TODO flat frame list
class ChunkAnimation : public FileChunk<Model, Model> {
public:
	ChunkAnimation() : FileChunk("animation") {}
	void create() override {
		me = parent;
	}
	void read(Stream *f) override {
#if 1
		[[maybe_unused]] int version = f->read_int();


		auto meta = new MetaMove;
		me->_template->animator->meta = meta;

		// headers
		int num_anims = f->read_int();
		meta->move.resize(num_anims);
		Array<int> frame_offset;
		foreachi (auto &m, meta->move, i) {
			m.name = f->read_str();
			m.id = f->read_int();
			m.id = i;
			m.type = (AnimationType)f->read_char();
			m.frame0 = f->read_int();
			m.num_frames = f->read_int();
			m.frames_per_sec_const = f->read_float();
			m.frames_per_sec_factor = f->read_float();
			// flags
			m.inter_quad = f->read_bool();
			m.inter_loop = f->read_bool();
		}

		// vertex animation frames
		meta->num_frames_vertex = f->read_int();
		for (int s=0; s<4; s++) {
			int n_vert = 0;
			if (parent->_template->mesh_collider->phys)
				n_vert = parent->_template->mesh_collider->phys->vertex.num;
			if (s > 0)
				n_vert = parent->mesh[s - 1]->vertex.num;
			meta->mesh[s].dpos.resize(meta->num_frames_vertex * n_vert);
		}
		for (int fr=0; fr<meta->num_frames_vertex; fr++) {
			/*fr.duration =*/ f->read_float();
			for (int s=0; s<4; s++) {
				int np = parent->_template->mesh_collider->phys->vertex.num;
				if (s >= 1)
					np = parent->mesh[s - 1]->vertex.num;
				int num_vertices = f->read_int();
				for (int j=0;j<num_vertices;j++) {
					int vertex_index = f->read_int();
					f->read_vector(&meta->mesh[s].dpos[fr * np + vertex_index]);
				}
			}
		}

		auto sk = parent->_template->skeleton;

		meta->num_frames_skeleton = f->read_int();
		if (meta->num_frames_skeleton > 0){
			meta->skel_dpos.resize(meta->num_frames_skeleton * sk->bones.num);
			meta->skel_ang.resize(meta->num_frames_skeleton * sk->bones.num);
		}
		Array<bool> var_delta_pos;
		int num_bones = f->read_int();
		var_delta_pos.resize(num_bones);
		for (int j=0; j<num_bones; j++)
			var_delta_pos[j] = f->read_bool();
		for (int fr=0; fr<meta->num_frames_skeleton; fr++) {
			/*fr.duration =*/ f->read_float();

			for (int j=0; j<num_bones; j++) {
				vec3 v;
				f->read_vector(&v);
				meta->skel_ang[fr * sk->bones.num + j] = quaternion::rotation_v(v);
			}

			for (int j=0; j<num_bones; j++)
				if (var_delta_pos[j])
					f->read_vector(&meta->skel_dpos[fr * sk->bones.num + j]);
		}
#endif
	}
	void write(Stream *f) override {}
};

/*class ModelEffectData;

class ChunkEffect : public FileChunk<Model, ModelEffectData> {
public:
	ChunkEffect() : FileChunk("effect") {}
	void create() override {
		parent->_template->fx.add(ModelEffectData());
		me = &parent->_template->fx.back();
	}
	void read(Stream *f) override {
		string fxkind = f->read_str();
		me->type = -1;
		if (fxkind == "script") {
			me->type = FX_TYPE_SCRIPT;
			me->vertex = f->read_int();
			me->filename = f->read_str();
			f->read_str();
		} else if (fxkind == "light") {
			me->type = FX_TYPE_LIGHT;
			me->vertex = f->read_int();
			me->radius = f->read_float();
			me->am = file_read_color4i(f);
			me->di = file_read_color4i(f);
			me->sp = file_read_color4i(f);
		} else if (fxkind == "sound") {
			me->type = FX_TYPE_SOUND;
			me->vertex = f->read_int();
			me->radius = f->read_float();
			me->speed = f->read_float();
			me->filename = f->read_str();
		} else if (fxkind == "forcefield") {
			me->type = FX_TYPE_FORCEFIELD;
			me->vertex = f->read_int();
			me->radius = f->read_float();
			f->read_float();
			f->read_bool();
		} else {
			//throw FormatError("unknown effect: " + fxkind);
		}
	}
	void write(Stream *f) override {}
};*/

class ChunkScript : public FileChunk<Model, Model> {
public:
	ChunkScript() : FileChunk("script") {}
	void create() override {
		me = parent;
	}
	void read(Stream *f) override {
		f->read_str(); // filename
		int n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();

		n = f->read_int();
		for (int i=0; i<n; i++) {
			TemplateDataScriptVariable v;
			v.name = f->read_str().lower().replace("_", "");
			v.value = f->read_str();
		}
	}
	void write(Stream *f) override {}
};

class ChunkModel : public FileChunk<Model, Model> {
public:
	ChunkModel() : FileChunk("model") {}
	void define_children() override {
		add_child(new ChunkMeta);
		add_child(new ChunkMaterial);
		add_child(new ChunkMesh);
		add_child(new ChunkPhysicalMesh);
		add_child(new ChunkSkeleton);
		add_child(new ChunkAnimation);
		add_child(new ChunkScript);
//		add_child(new ChunkEffect);
		add_child(new ChunkOldMeta);
	}
	void read(Stream *f) override {}
	void write(Stream *f) override {}
};


class ModelParser : public ChunkedFileParser {
public:
	explicit ModelParser(ModelManager *_model_manager) : ChunkedFileParser(8) {
		model_manager = _model_manager;
		_model_parser_tria_mesh_count = 0;
		set_base(new ChunkModel);
	}
	void on_notify() override {}
	void on_unhandled() override {
		//session->set_message("unhandled chunk " + context.str());
	}
	void on_error(const string &message) override {
		//session->error(message);
	}
	void on_warn(const string &message) override {}
	void on_info(const string &message) override {}

	ModelManager *model_manager;
};

}

xfer<Model> fancy_copy(Model *orig) {
	Model *clone = new Model();
	//clone->owner = new Entity3D(Entity::Type::ENTITY3D);
	return orig->copy(clone);
}


ModelManager::ModelManager(ResourceManager *_resource_manager, yrenderer::MaterialManager *_material_manager) {
	resource_manager = _resource_manager;
	material_manager = _material_manager;
}

xfer<Model> ModelManager::load(const Path &_filename) {
	if (_filename == "")
		return nullptr;
	auto filename = engine.object_dir | _filename.with(".model");
	for (auto *o: originals)
		if (o->_template->filename == filename) {
			return fancy_copy(o);
		}

	msg_write("loading " + filename.str());
	auto m = new Model();
	m->_template = new ModelTemplate(m);
	m->_template->filename = filename;
	m->_template->solid_body = new SolidBody;
	m->_template->mesh_collider = new MeshCollider;
	m->_template->animator = new Animator;
	m->_template->skeleton = new Skeleton;
	m->_template->vertex_shader_module = "default";

	modelmanager::ModelParser p(this);
	p.read(filename, m);

	// remove unneeded components
	/*if (m->_template->mesh_collider->phys->balls.num + ... == 0) {
		delete m->_template->mesh_collider;
		m->_template->mesh_collider = nullptr;
	}*/
	if (!m->_template->solid_body->active and !m->_template->solid_body->passive) {
		delete m->_template->solid_body;
		m->_template->solid_body = nullptr;
	}
	if (!m->_template->animator->meta) {
		delete m->_template->animator;
		m->_template->animator = nullptr;
	}
	if (m->_template->skeleton->bones.num == 0) {
		delete m->_template->skeleton;
		m->_template->skeleton = nullptr;
	}


	// do some post processing...
	AppraiseDimensions(m);

	for (int i=0; i<MODEL_NUM_MESHES; i++)
		m->mesh[i]->post_process(m->_template->animator);

	PostProcessPhys(m, m->_template->mesh_collider->phys);

	if (m->_template->animator)
		m->_template->vertex_shader_module = "animated";




	m->is_copy = false;
	m->reset_data();


	//m->load(filename);
	originals.add(m);
	return fancy_copy(m);
}


yrenderer::MaterialManager *chunked_file_parser_get_material_manager(ChunkedFileParser *p) {
	return static_cast<modelmanager::ModelParser*>(p)->model_manager->material_manager;
}
ResourceManager *chunked_file_parser_get_resource_manager(ChunkedFileParser *p) {
	return static_cast<modelmanager::ModelParser*>(p)->model_manager->resource_manager;
}

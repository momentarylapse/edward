/*
 * ModelManager.cpp
 *
 *  Created on: 19.01.2020
 *      Author: michi
 */

#include "ModelManager.h"
#include "Model.h"
#include "../y/EngineData.h"
#include "../lib/math/complex.h"
#include "../lib/kaba/kaba.h"
#include "../lib/config.h"
#ifdef _X_ALLOW_X_
#include "../helper/ResourceManager.h"
#include "../plugins/PluginManager.h"
#else
#include "ResourceManager.h"
#endif
#include "../lib/file/file.h"
#include "../lib/xfile/chunked.h"
#include "../lib/nix/nix.h"
#include "../meta.h"
#include "Material.h"

Array<Model*> ModelManager::originals;


File *load_file_x(const Path &filename, int &version) {

	File *f = FileOpen(filename);
	char c = f->read_char();
	if (c == 'b') {
		version = f->read_word();
		return f;
	} else if (c == 't') {
		delete f;
		f = FileOpenText(filename);
		f->read_char();
		version = f->read_word();
		return f;
	} else {
		throw Exception("File format unreadable!");
	}
	return nullptr;
}


color file_read_color4i(File *f) {
	int a = f->read_int();
	int r = f->read_int();
	int g = f->read_int();
	int b = f->read_int();
	return color((float)a/255.0f, (float)r/255.0f, (float)g/255.0f, (float)b/255.0f);
}

ivec4 read_ivec4(File *f) {
	ivec4 v;
	v.i = f->read_int();
	v.j = f->read_int();
	v.k = f->read_int();
	v.l = f->read_int();
	return v;
}

vec4 read_vec4(File *f) {
	vec4 v;
	v.x = f->read_float();
	v.y = f->read_float();
	v.z = f->read_float();
	v.w = f->read_float();
	return v;
}

void write_ivec4(File *f, const ivec4 &v) {
	f->write_int(v.i);
	f->write_int(v.j);
	f->write_int(v.k);
	f->write_int(v.l);
}

void write_vec4(File *f, const vec4 &v) {
	f->write_float(v.x);
	f->write_float(v.y);
	f->write_float(v.z);
	f->write_float(v.w);
}



vector get_normal_by_index(int index) {
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vector( cos(wxy) * swz, sin(wxy) * swz, cwz);
}

void AppraiseDimensions(Model *m);
void PostProcessPhys(Model *m, PhysicalMesh *s);

namespace modelmanager {

class ChunkMeta : public FileChunk<Model, Model> {
public:
	ChunkMeta() : FileChunk("meta") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		int version = f->read_int();

		f->read_vector(&me->prop.min);
		f->read_vector(&me->prop.max);
		me->prop.radius = f->read_float();

		// physics
		me->physics_data.mass = f->read_float();
		for (int i=0;i<9;i++)
			me->physics_data.theta_0.e[i] = f->read_float();
		me->physics_data.active = f->read_bool();
		me->physics_data.passive = f->read_bool();
	}
	void write(File *f) override {}
};

class ChunkOldMeta : public FileChunk<Model, Model> {
public:
	ChunkOldMeta() : FileChunk("xxx1") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		// Object Data
		me->script_data.name = f->read_str();
		me->script_data.description = f->read_str();

		// Inventary
		me->script_data.inventary.resize(f->read_int());
		for (int i=0;i<me->script_data.inventary.num;i++)
			me->script_data.inventary[i] = ModelManager::load(f->read_str());
	}
	void write(File *f) override {}
};

class ChunkMaterial : public FileChunk<Model, Material> {
public:
	ChunkMaterial() : FileChunk("material") {}
	void create() override {
		me = new Material;
		parent->material.add(me);
	}
	void read(File *f) override {
		Material *m = LoadMaterial(f->read_str());
		*me = *m;
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

		auto alpha_mode = (TransparencyMode)f->read_int();
		if (alpha_mode != TransparencyMode::DEFAULT) {
			me->alpha.mode = alpha_mode;
			me->alpha.source = (nix::Alpha)f->read_int();
			me->alpha.destination = (nix::Alpha)f->read_int();
			me->alpha.factor = f->read_float();
			me->alpha.z_buffer = f->read_bool();
		} else {
			f->read_int();
			f->read_int();
			f->read_float();
			f->read_bool();
		}
		int nt = f->read_int();
		if (nt > me->textures.num)
			me->textures.resize(nt);
		for (int t=0;t<nt;t++) {
			Path fn = f->read_str();
			if (!fn.is_empty())
				me->textures[t] = ResourceManager::load_texture(fn);
		}
	}
	void write(File *f) override {}
};

static int _model_parser_tria_mesh_count;

class ChunkMesh : public FileChunk<Model, Mesh> {
public:
	ChunkMesh() : FileChunk("mesh") {}
	void create() override {
		me = new Mesh;
		parent->mesh[_model_parser_tria_mesh_count ++] = me;
	}
	void read(File *f) override {
		int version = f->read_int();
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
			// triangles
			sub->num_triangles = f->read_int();
			sub->triangle_index.resize(sub->num_triangles * 3);
			sub->skin_vertex.resize(parent->material[m]->textures.num * sub->num_triangles * 6);
			sub->normal.resize(sub->num_triangles * 3);
			// vertices
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->triangle_index[i] = f->read_int();
			// skin vertices
			//msg_write(format("%d  %d", parent->material[m]->textures.num, sub->num_triangles));
			for (int i=0;i<parent->material[m]->textures.num * sub->num_triangles * 3;i++){
				int sv = f->read_int();
				sub->skin_vertex[i * 2    ] = skin_vert[sv].x;
				sub->skin_vertex[i * 2 + 1] = skin_vert[sv].y;
			}
			// normals
			for (int i=0;i<sub->num_triangles * 3;i++)
				sub->normal[i] = get_normal_by_index(f->read_word());

			sub->force_update = true;
			sub->vertex_buffer = NULL;
		}
	}
	void write(File *f) override {}
};

class ChunkPhysicalMesh : public FileChunk<Model, PhysicalMesh> {
public:
	ChunkPhysicalMesh() : FileChunk("physmesh") {}
	void define_children() override {
	}
	void create() override {
		me = new PhysicalMesh;
		parent->phys = me;
	}
	void read(File *f) override {
		int version = f->read_int();

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
	void write(File *f) override {}
};


class ChunkSkeleton : public FileChunk<Model, Model> {
public:
	ChunkSkeleton() : FileChunk("skeleton") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		int version = f->read_int();
		me->bone.resize(f->read_int());
		for (auto &b: me->bone) {
			f->read_vector(&b.delta_pos);
			b.parent = f->read_int();
			string filename = f->read_str();
			b.model = ModelManager::load(filename);
		}
		f->read_str();
	}
	void write(File *f) override {}
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
	void read(File *f) override {
#if 1
		int version = f->read_int();

		me->anim.meta = new MetaMove;
		auto meta = me->anim.meta;

		// headers
		int num_anims = f->read_int();
		meta->move.resize(num_anims);
		Array<int> frame_offset;
		for (auto &m: meta->move) {
			m.name = f->read_str();
			m.id = f->read_int();
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
			if (parent->phys)
				n_vert = parent->phys->vertex.num;
			if (s > 0)
				n_vert = parent->mesh[s - 1]->vertex.num;
			meta->mesh[s].dpos.resize(meta->num_frames_vertex * n_vert);
		}
		for (int fr=0; fr<meta->num_frames_vertex; fr++) {
			/*fr.duration =*/ f->read_float();
			for (int s=0; s<4; s++) {
				int np = parent->phys->vertex.num;
				if (s >= 1)
					np = parent->mesh[s - 1]->vertex.num;
				int num_vertices = f->read_int();
				for (int j=0;j<num_vertices;j++) {
					int vertex_index = f->read_int();
					f->read_vector(&meta->mesh[s].dpos[fr * np + vertex_index]);
				}
			}
		}


		meta->num_frames_skeleton = f->read_int();
		if (meta->num_frames_skeleton > 0){
			meta->skel_dpos.resize(meta->num_frames_skeleton * parent->bone.num);
			meta->skel_ang.resize(meta->num_frames_skeleton * parent->bone.num);
		}
		Array<bool> var_delta_pos;
		int num_bones = f->read_int();
		var_delta_pos.resize(num_bones);
		for (int j=0; j<num_bones; j++)
			var_delta_pos[j] = f->read_bool();
		for (int fr=0; fr<meta->num_frames_skeleton; fr++) {
			/*fr.duration =*/ f->read_float();

			for (int j=0; j<num_bones; j++) {
				vector v;
				f->read_vector(&v);
				meta->skel_ang[fr * parent->bone.num + j] = quaternion::rotation_v(v);
			}

			for (int j=0; j<num_bones; j++)
				if (var_delta_pos[j])
					f->read_vector(&meta->skel_dpos[fr * parent->bone.num + j]);
		}
#endif
	}
	void write(File *f) override {}
};

class ChunkEffect : public FileChunk<Model, ModelEffectData> {
public:
	ChunkEffect() : FileChunk("effect") {}
	void create() override {
		parent->_template->fx.add(ModelEffectData());
		me = &parent->_template->fx.back();
	}
	void read(File *f) override {
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
	void write(File *f) override {}
};

class ChunkScript : public FileChunk<Model, Model> {
public:
	ChunkScript() : FileChunk("script") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		me->_template->script_filename = f->read_str();
		me->script_data.var.resize(f->read_int());
		for (int i=0;i<me->script_data.var.num;i++)
			me->script_data.var[i] = f->read_float();

		int n = f->read_int();
		for (int i=0; i<n; i++) {
			TemplateDataScriptVariable v;
			v.name = f->read_str().lower().replace("_", "");
			v.value = f->read_str();
			me->_template->variables.add(v);
		}
	}
	void write(File *f) override {}
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
		add_child(new ChunkEffect);
		add_child(new ChunkOldMeta);
	}
	void read(File *f) override {}
	void write(File *f) override {}
};


class ModelParser : public ChunkedFileParser {
public:
	ModelParser() : ChunkedFileParser(8) {
		_model_parser_tria_mesh_count = 0;
		set_base(new ChunkModel);
	}
	void on_notify() override {}
	void on_unhandled() override {
		//ed->set_message("unhandled chunk " + context.str());
	}
	void on_error(const string &message) override {
		//ed->error_box(message);
	}
	void on_warn(const string &message) override {}
	void on_info(const string &message) override {}
};

}



Model* fancy_copy(Model *m, const Path &_script) {
	Model *c = nullptr;
#ifdef _X_ALLOW_X_
	Path script = m->_template->script_filename;
	if (!_script.is_empty())
		script = _script;
	//msg_write(format("MODEL  %s   %s", m->_template->filename, script));
	if (!script.is_empty())
		c = (Model*)plugin_manager.create_instance(script, "y.Model", m->_template->variables);
#endif
	if (!c)
		c = new Model();
	return m->copy(c);
}

Model* ModelManager::load(const Path &_filename) {
	return loadx(_filename, "");
}

Model* ModelManager::loadx(const Path &_filename, const Path &_script) {
	if (_filename == "")
		return nullptr;
	auto filename = engine.object_dir << _filename.with(".model");
	for (auto *o: originals)
		if (o->_template->filename == filename) {
			return fancy_copy(o, _script);
		}

	Model *m = new Model();
	m->_template = new ModelTemplate(m);
	m->_template->filename = filename;
	msg_write("loading " + filename.str());
	modelmanager::ModelParser p;
	p.read(filename, m);



	// do some post processing...
	AppraiseDimensions(m);

	for (int i=0; i<MODEL_NUM_MESHES; i++)
		m->mesh[i]->post_process(m->uses_bone_animations());

	PostProcessPhys(m, m->phys);




	// skeleton
	if (m->bone.num > 0) {
		m->anim.dmatrix.resize(m->bone.num);
		for (int i=0; i<m->bone.num; i++) {
			m->bone[i].rest_pos = m->get_bone_rest_pos(i);
			m->anim.dmatrix[i] = matrix::translation(m->bone[i].rest_pos);
		}
	}



	m->is_copy = false;
	m->reset_data();


	//m->load(filename);
	originals.add(m);
	return fancy_copy(m, _script);
}

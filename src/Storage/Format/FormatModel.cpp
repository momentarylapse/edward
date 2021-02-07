/*
 * FormatModel.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModel.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/ModelMesh.h"
#include "../../Data/Model/ModelPolygon.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Edward.h"
#include "../../x/ModelManager.h"
#include "../../lib/xfile/chunked.h"

FormatModel::FormatModel() : TypedFormat<DataModel>(FD_MODEL, "model", _("Model"), Flag::CANONICAL_READ_WRITE) {
}

const bool write_external_edit_file = false;


void update_model_script_data(DataModel::MetaData &m);

bool DataModelAllowUpdating = true;


int get_normal_index(vector &n) {
	int nxy, nz;
	if ((n.x == 0) and (n.y == 0)) {
		nxy = 0;
		nz = (n.z < 0) ? 255 : 0;
	} else {
		nxy = (int)(atan2(n.y, n.x) / pi / 2 * 255.0f) & 255;
		nz = (int)(acos(n.z) / pi * 255.0f) & 255;
	}
	return nxy + 256 * nz;
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



char read_first_char(const Path &filename) {
	File *f = FileOpen(filename);
	return f->read_char();
}


Set<int> get_all_poly_vert(DataModel *m) {
	Set<int> all_vert;
	for (auto &p: m->phys_mesh->polygon)
		for (auto &f: p.side)
			all_vert.add(f.vertex);
	return all_vert;
}

Array<Set<int>> split_conv_polyhedra(DataModel *m) {
	auto all_vert = get_all_poly_vert(m);

	Array<Set<int>> surf;

	while(all_vert.num > 0) {
		msg_write("----surf");
		Set<int> cur;
		cur.add(all_vert.pop());
		int n = 0;
		while (cur.num > n) {
			n = cur.num;

			for (auto &e: m->phys_mesh->edge) {
				if (cur.contains(e.vertex[0]) and all_vert.contains(e.vertex[1])) {
					cur.add(e.vertex[1]);
					all_vert.erase(e.vertex[1]);
				}
				if (cur.contains(e.vertex[1]) and all_vert.contains(e.vertex[0])) {
					cur.add(e.vertex[0]);
					all_vert.erase(e.vertex[0]);
				}
			}
		}
		msg_write(ia2s(cur));
		surf.add(cur);
	}
	return surf;
}

Array<ModelPolygon> conv_poly_poly(DataModel *m, const Set<int> &v) {
	Array<ModelPolygon> poly;
	for (auto &p: m->phys_mesh->polygon)
		if (v.contains(p.side[0].vertex))
			poly.add(p);
	return poly;
}


// model
//   meta
//   material[]
//   physmesh
//     polyhedron[]
//     cylinder[]
//     ball[]
//   triamesh[]
//     vertices
//     uv
//     triangles
//   skeleton
//   animation
//   script
//   effects
//   editor
//     polymesh

// TODO: bone info: bone0:i12, bone1:i12, fraction:i8

class ChunkMeta : public FileChunk<DataModel, DataModel> {
public:
	ChunkMeta() : FileChunk("meta") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		int version = f->read_int();

		vector temp;
		f->read_vector(&temp);
		f->read_vector(&temp);
		f->read_float();

		// physics
		me->meta_data.mass = f->read_float();
		for (int i=0;i<9;i++)
			me->meta_data.inertia_tensor.e[i] = f->read_float();
		me->meta_data.active_physics = f->read_bool();
		me->meta_data.passive_physics = f->read_bool();
	}
	void write(File *f) override {
		// version
		f->write_int(0);

		vector _min, _max;
		me->getBoundingBox(_min, _max);
		f->write_vector(&_min);
		f->write_vector(&_max);
		f->write_float(me->getRadius());

		// physics
		f->write_float(me->meta_data.mass);
		for (int i=0;i<9;i++)
			f->write_float(me->meta_data.inertia_tensor.e[i]);
		f->write_bool(me->meta_data.active_physics);
		f->write_bool(me->meta_data.passive_physics);
	}
};

class ChunkOldMeta : public FileChunk<DataModel, DataModel> {
public:
	ChunkOldMeta() : FileChunk("xxx1") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		// Object Data
		me->meta_data.name = f->read_str();
		me->meta_data.description = f->read_str();

		// Inventary
		me->meta_data.inventary.resize(f->read_int());
		for (int i=0;i<me->meta_data.inventary.num;i++)
			me->meta_data.inventary[i] = f->read_str();
	}
	void write(File *f) override {
		// object data
		f->write_str(me->meta_data.name);
		f->write_str(me->meta_data.description);

		// inventory
		f->write_int(me->meta_data.inventary.num);
		for (int i=0;i<me->meta_data.inventary.num;i++)
			f->write_str(me->meta_data.inventary[i].str());
	}
};

class ChunkMaterial : public FileChunk<DataModel, ModelMaterial> {
public:
	ChunkMaterial() : FileChunk("material") {}
	void create() override {
		me = new ModelMaterial;
		parent->material.add(me);
	}
	void read(File *f) override {
		me->filename = f->read_str();
		me->col.user = f->read_bool();
		read_color_argb(f, me->col.albedo);
		read_color_argb(f, me->col.emission);
		me->col.metal = f->read_float();
		me->col.roughness = f->read_float();

		me->alpha.mode = f->read_int();
		me->alpha.user = (me->alpha.mode != TransparencyModeDefault);
		me->alpha.source = f->read_int();
		me->alpha.destination = f->read_int();
		me->alpha.factor = f->read_float();
		me->alpha.zbuffer = f->read_bool();
		int n = f->read_int();
		me->texture_levels.clear();
		for (int t=0;t<n;t++) {
			auto tl = new ModelMaterial::TextureLevel();
			tl->filename = f->read_str();
			me->texture_levels.add(tl);
		}
	}
	void write(File *f) override {
		f->write_str(me->filename.str());
		f->write_bool(me->col.user);
		write_color_argb(f, me->col.albedo);
		write_color_argb(f, me->col.emission);
		f->write_float(me->col.metal);
		f->write_float(me->col.roughness);
		f->write_int(me->alpha.user ? me->alpha.mode : TransparencyModeDefault);
		f->write_int(me->alpha.source);
		f->write_int(me->alpha.destination);
		f->write_float(me->alpha.factor);
		f->write_bool(me->alpha.zbuffer);
		f->write_int(me->texture_levels.num);
		for (int t=0;t<me->texture_levels.num;t++)
			f->write_str(me->texture_levels[t]->filename.str());
	}
};

static int _model_parser_tria_mesh_count;

class ChunkTriangleMesh : public FileChunk<DataModel, ModelSkin> {
public:
	ChunkTriangleMesh() : FileChunk("triamesh") {}
	void create() override {
		me = &parent->skin[1 + _model_parser_tria_mesh_count ++];
	}
	void read(File *f) override {
		// vertices
		int nv = f->read_int();
		me->vertex.resize(nv);
		for (int j=0;j<me->vertex.num;j++)
			f->read_vector(&me->vertex[j].pos);
		for (int j=0;j<me->vertex.num;j++)
			me->vertex[j].bone_index = f->read_int();
		for (int j=0;j<me->vertex.num;j++)
			me->vertex[j].normal_dirty = false;//true;

		// skin vertices
		Array<vector> skin_vert;
		int nsv = f->read_int();
		skin_vert.resize(nsv);
		for (int j=0;j<skin_vert.num;j++) {
			skin_vert[j].x = f->read_float();
			skin_vert[j].y = f->read_float();
		}



		// triangles (subs)
		me->sub.resize(parent->material.num);
		for (int m=0; m<parent->material.num; m++) {
			int ntria = f->read_int();
			me->sub[m].triangle.resize(ntria);
			// vertex
			for (int j=0;j<me->sub[m].triangle.num;j++)
				for (int k=0;k<3;k++)
					me->sub[m].triangle[j].vertex[k] = f->read_int();
			// skin vertex
			for (int tl=0;tl<parent->material[m]->texture_levels.num;tl++)
				for (int j=0;j<me->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++) {
						int svi = f->read_int();
						me->sub[m].triangle[j].skin_vertex[tl][k] = skin_vert[svi];
					}
			// normals
			for (int j=0;j<me->sub[m].triangle.num;j++) {
				for (int k=0;k<3;k++) {
					me->sub[m].triangle[j].normal_index[k] = (int)(unsigned short)f->read_word();
					me->sub[m].triangle[j].normal[k] = get_normal_by_index(me->sub[m].triangle[j].normal_index[k]);
				}
				me->sub[m].triangle[j].normal_dirty = false;
			}
		}
	}
	void write(File *f) override {

		// verices
		f->write_int(me->vertex.num);
		for (ModelVertex &v: me->vertex)
			f->write_vector(&v.pos);
		for (ModelVertex &v: me->vertex)
			f->write_int(v.bone_index);

		// skin vertices
		int num_skin_v = 0;
		for (int m=0;m<parent->material.num;m++)
			num_skin_v += me->sub[m].triangle.num * parent->material[m]->texture_levels.num * 3;
		f->write_int(num_skin_v);
		for (int m=0;m<parent->material.num;m++)
			for (int tl=0;tl<parent->material[m]->texture_levels.num;tl++)
				for (int j=0;j<me->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++) {
						f->write_float(me->sub[m].triangle[j].skin_vertex[tl][k].x);
						f->write_float(me->sub[m].triangle[j].skin_vertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<parent->material.num;m++) {
			ModelSubSkin *sub = &me->sub[m];

			// triangles
			f->write_int(sub->triangle.num);

			// vertex index
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++)
					f->write_int(sub->triangle[j].vertex[k]);

			// skin index
			for (int tl=0;tl<parent->material[m]->texture_levels.num;tl++)
				for (int j=0;j<sub->triangle.num;j++)
					for (int k=0;k<3;k++)
						f->write_int(svi ++);

			// normal
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++) {
					if (DataModelAllowUpdating)
						sub->triangle[j].normal_index[k] = get_normal_index(sub->triangle[j].normal[k]);
					f->write_word(sub->triangle[j].normal_index[k]);
				}
		}
	}
};

class ChunkPolygonMesh : public FileChunk<DataModel, ModelMesh> {
public:
	ChunkPolygonMesh() : FileChunk("polymesh") {}
	void define_children() override {
	}
	void create() override {
		me = parent->mesh;
	}
	void read(File *f) override {

		foreachi(ModelVertex &v, parent->skin[1].vertex, i)
			parent->addVertex(v.pos, v.bone_index, v.normal_mode);

		// polygons
		int num_poly = f->read_int();
		for (int j=0; j<num_poly; j++) {
			ModelPolygon t;
			t.is_selected = false;
			t.triangulation_dirty = true;
			int num_faces = f->read_word();
			t.material = f->read_word();
			t.smooth_group = (int)f->read_word();
			if (t.smooth_group == 65535)
				t.smooth_group = -1;
			t.side.resize(num_faces);
			for (int k=0; k<num_faces; k++) {
				t.side[k].vertex = f->read_int();
				for (int l=0;l<parent->material[t.material]->texture_levels.num;l++) {
					t.side[k].skin_vertex[l].x = f->read_float();
					t.side[k].skin_vertex[l].y = f->read_float();
				}
			}
			t.normal_dirty = true;
			me->polygon.add(t);
		}

		me->build_topology();
	}
	void write(File *f) override {

		// polygons
		f->write_int(me->polygon.num);
		for (auto &t: me->polygon) {
			f->write_word(t.side.num);
			f->write_word(t.material);
			f->write_word(t.smooth_group);
			for (auto &ss: t.side) {
				f->write_int(ss.vertex);
				for (int l=0;l<parent->material[t.material]->texture_levels.num;l++) {
					f->write_float(ss.skin_vertex[l].x);
					f->write_float(ss.skin_vertex[l].y);
				}
			}
		}
	}
};

class ChunkPhysicalMesh : public FileChunk<DataModel, ModelMesh> {
public:
	ChunkPhysicalMesh() : FileChunk("physmesh") {}
	void define_children() override {
	}
	void create() override {
		me = parent->phys_mesh;
	}
	void read(File *f) override {
		int version = f->read_int();

		// vertices
		me->vertex.resize(f->read_int());
		for (int j=0;j<me->vertex.num;j++)
			f->read_vector(&me->vertex[j].pos);
		for (int j=0;j<me->vertex.num;j++)
			me->vertex[j].bone_index = f->read_int();

		msg_write("phys vert: " + i2s(me->vertex.num));

		// triangles
		f->read_int();

		// balls
		me->ball.resize(f->read_int());
		for (auto &b: me->ball) {
			b.index = f->read_int();
			b.radius = f->read_float();
		}

		// polyhedra
		int num_polys = f->read_int();
		for (int i=0; i<num_polys; i++) {
			msg_write("POLYHEDRON! " + format("%d/%d", i, num_polys));

			Array<Array<int>> vv;
			int num_faces = f->read_int();
			msg_write("  faces: " + i2s(num_faces));
			for (int k=0;k<num_faces;k++) {
				int nv = f->read_int();
				Array<int> vertex;
				for (int l=0;l<nv;l++) {
					vertex.add(f->read_int());
				}
				msg_write(ia2s(vertex));
				vv.add(vertex);
			}

			for (auto &v: vv) {
				Array<vector> sv;
				sv.resize(v.num * MATERIAL_MAX_TEXTURES);//data->material[0]->texture_levels.num);
				//msg_write(ia2s(_vv));
				try {
					msg_write(" + poly " + ia2s(v));
					me->_add_polygon(v, 0, sv);
				} catch(GeometryException &e) {
					msg_error(e.message);
				}
			}
		}

		// cylinders
		int n = f->read_int();
		for (int i=0; i<n; i++) {
			ModelCylinder c;
			c.index[0] = f->read_int();
			c.index[1] = f->read_int();
			c.radius = f->read_float();
			c.round = f->read_bool();
			me->cylinder.add(c);
		}
	}
	void write(File *f) override {
		// version
		f->write_int(0);

		// vertices
		f->write_int(me->vertex.num);
		for (int j=0;j<me->vertex.num;j++)
			f->write_vector(&me->vertex[j].pos);
		// FIXME
		for (int j=0;j<me->vertex.num;j++)
			f->write_int(me->vertex[j].bone_index);

		// triangles
		f->write_int(0);

		// balls
		f->write_int(me->ball.num);
		for (auto &b: me->ball) {
			f->write_int(b.index);
			f->write_float(b.radius);
		}

		// polyhedra
		auto surf = split_conv_polyhedra(parent);
		f->write_int(surf.num);
		for (auto &pp: surf) {
			auto p = conv_poly_poly(parent, pp);

			f->write_int(p.num);
			for (int k=0;k<p.num;k++) {
				f->write_int(p[k].side.num);
				for (int l=0;l<p[k].side.num;l++)
					f->write_int(p[k].side[l].vertex);
			}
		}

		// cylinders
		f->write_int(me->cylinder.num);
		for (auto &c: me->cylinder) {
			f->write_int(c.index[0]);
			f->write_int(c.index[1]);
			f->write_float(c.radius);
			f->write_bool(c.round);
		}

	}
};


class ChunkSkeleton : public FileChunk<DataModel, DataModel> {
public:
	ChunkSkeleton() : FileChunk("skeleton") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		int version = f->read_int();
		me->bone.resize(f->read_int());
		for (auto &b: me->bone) {
			f->read_vector(&b.pos);
			b.parent = f->read_int();
			if ((b.parent < 0) or (b.parent >= me->bone.num))
				b.parent = -1;
			if (b.parent >= 0)
				b.pos += me->bone[b.parent].pos;
			b.model_file = f->read_str();
			b.const_pos = false;
			b.is_selected = b.m_old = false;
		}
		f->read_str();
	}
	void write(File *f) override {
		f->write_int(0);
		f->write_int(me->bone.num);
		for (auto &b: me->bone) {
			if (b.parent >= 0) {
				vector dpos = b.pos - me->bone[b.parent].pos;
				f->write_vector(&dpos);
			} else {
				f->write_vector(&b.pos);
			}
			f->write_int(b.parent);
			f->write_str(b.model_file.str());
		}
		f->write_str("");
	}
};

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


// TODO later write skeleton animations into separate file...
// TODO flat frame list
class ChunkAnimation : public FileChunk<DataModel, DataModel> {
public:
	ChunkAnimation() : FileChunk("animation") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		int version = f->read_int();

		// headers
		int num_anims = f->read_int();
		me->move.resize(num_anims);
		Array<int> frame_offset;
		for (auto &m: me->move) {
			m.name = f->read_str();
			m.id = f->read_int();
			m.type = f->read_char();
			frame_offset.add(f->read_int());
			m.frame.resize(f->read_int());
			m.frames_per_sec_const = f->read_float();
			m.frames_per_sec_factor = f->read_float();
			// flags
			m.interpolated_quadratic = f->read_bool();
			m.interpolated_loop = f->read_bool();
		}

		// vertex animation frames
		int n_frames_vert = f->read_int();
		Array<ModelFrame> frames_vert;
		frames_vert.resize(n_frames_vert);
		for (auto &fr: frames_vert) {
			fr.duration = f->read_float();
			for (int s=0; s<4; s++) {
				fr.skin[s].dpos.resize(me->skin[s].vertex.num);
				int num_vertices = f->read_int();
				for (int j=0;j<num_vertices;j++) {
					int vertex_index = f->read_int();
					f->read_vector(&fr.skin[s].dpos[vertex_index]);
				}
			}
		}


		int n_frames_skel = f->read_int();
		Array<bool> var_delta_pos;
		int num_bones = f->read_int();
		var_delta_pos.resize(num_bones);
		for (int j=0; j<num_bones; j++)
			var_delta_pos[j] = f->read_bool();
		Array<ModelFrame> frames_skel;
		frames_skel.resize(n_frames_skel);
		for (auto &fr: frames_skel) {
			fr.duration = f->read_float();
			fr.skel_dpos.resize(num_bones);
			fr.skel_ang.resize(num_bones);
			for (int j=0;j<num_bones;j++)
				f->read_vector(&fr.skel_ang[j]);
			for (int j=0;j<num_bones;j++)
				if (var_delta_pos[j])
					f->read_vector(&fr.skel_dpos[j]);
		}

		import_animations(parent, frames_vert, frames_skel, frame_offset);
	}
	void write(File *f) override {
		f->write_int(0);

		// headers
		int num_moves = 0;
		for (auto &m: me->move)
			if (m.frame.num > 0)
				num_moves ++;
		f->write_int(num_moves);//me->move.num);
		int n_frames_vert = 0;
		int n_frames_skel = 0;
		foreachi (auto &m, me->move, index) {
			if (m.frame.num == 0)
				continue;
			f->write_str(m.name);
			f->write_int(m.id);
			f->write_char(m.type);
			// offset
			if (m.type == MOVE_TYPE_VERTEX)
				f->write_int(n_frames_vert);
			else //if (m.type == MOVE_TYPE_SKELETAL)
				f->write_int(n_frames_skel);
			f->write_int(m.frame.num);
			f->write_float(m.frames_per_sec_const);
			f->write_float(m.frames_per_sec_factor);
			// flags
			f->write_bool(m.interpolated_quadratic);
			f->write_bool(m.interpolated_loop);

			if (m.type == MOVE_TYPE_VERTEX)
				n_frames_vert += m.frame.num;
			else //if (m.type == MOVE_TYPE_SKELETAL)
				n_frames_skel += m.frame.num;
		}


		// vertex animation frames
		f->write_int(n_frames_vert);
		for (auto &m: me->move)
			if (m.type == MOVE_TYPE_VERTEX)
				for (auto &fr: m.frame) {
					f->write_float(fr.duration);
					for (int s=0; s<4; s++) {
						// compress (only write != 0)
						int num_vertices = 0;
						for (int j=0;j<me->skin[s].vertex.num;j++)
							if (fr.skin[s].dpos[j] != v_0)
								num_vertices ++;
						f->write_int(num_vertices);
						for (int j=0;j<me->skin[s].vertex.num;j++)
							if (fr.skin[s].dpos[j] != v_0) {
								f->write_int(j);
								f->write_vector(&fr.skin[s].dpos[j]);
							}
					}
				}

		// skeleton animation frames
		f->write_int(n_frames_skel);
		f->write_int(parent->bone.num);
		for (auto &b: parent->bone)
			f->write_bool(b.parent < 0);
		for (auto &m: me->move)
			if (m.type == MOVE_TYPE_SKELETAL)
				for (auto &fr: m.frame) {
					f->write_float(fr.duration);
					for (int j=0;j<me->bone.num;j++)
						f->write_vector(&fr.skel_ang[j]);
					for (int j=0;j<me->bone.num;j++)
						if (me->bone[j].parent < 0)
							f->write_vector(&fr.skel_dpos[j]);
				}
	}
};

class ChunkEffect : public FileChunk<DataModel, ModelEffect> {
public:
	ChunkEffect() : FileChunk("effect") {}
	void create() override {
		parent->fx.add(ModelEffect());
		me = &parent->fx.back();
	}
	void read(File *f) override {
		string fxkind = f->read_str();
		me->type = -1;
		if (fxkind == "script") {
			me->type = FX_TYPE_SCRIPT;
			me->vertex = f->read_int();
			me->file = f->read_str();
			f->read_str();
		} else if (fxkind == "light") {
			me->type = FX_TYPE_LIGHT;
			me->vertex = f->read_int();
			me->size = f->read_float();
			for (int j=0;j<3;j++)
				read_color_argb(f,me->colors[j]);
		} else if (fxkind == "sound") {
			me->type = FX_TYPE_SOUND;
			me->vertex = f->read_int();
			me->size = f->read_float();
			me->speed = f->read_float();
			me->file = f->read_str();
		} else if (fxkind == "forcefield") {
			me->type = FX_TYPE_FORCEFIELD;
			me->vertex = f->read_int();
			me->size = f->read_float();
			me->intensity = f->read_float();
			me->inv_quad = f->read_bool();
		} else {
			throw FormatError("unknown effect: " + fxkind);
		}
	}
	void write(File *f) override {
		if (me->type == FX_TYPE_SCRIPT) {
			f->write_str("script");
			f->write_int(me->vertex);
			f->write_str(me->file.str());
			f->write_str("");
		} else if (me->type == FX_TYPE_LIGHT) {
			f->write_str("light");
			f->write_int(me->vertex);
			f->write_float(me->size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, me->colors[nc]);
		} else if (me->type == FX_TYPE_SOUND) {
			f->write_str("sound");
			f->write_int(me->vertex);
			f->write_float(me->size);
			f->write_float(me->speed);
			f->write_str(me->file.str());
		} else if (me->type == FX_TYPE_FORCEFIELD) {
			f->write_str("forcefield");
			f->write_int(me->vertex);
			f->write_float(me->size);
			f->write_float(me->intensity);
			f->write_bool(me->inv_quad);
		} else {
			f->write_str("???");
		}
	}
};

class ChunkScript : public FileChunk<DataModel, DataModel> {
public:
	ChunkScript() : FileChunk("script") {}
	void create() override {
		me = parent;
	}
	void read(File *f) override {
		me->meta_data.script_file = f->read_str();
		me->meta_data.script_var.resize(f->read_int());
		for (int i=0;i<me->meta_data.script_var.num;i++)
			me->meta_data.script_var[i] = f->read_float();

		int n = f->read_int();
		for (int i=0; i<n; i++) {
			ModelScriptVariable v;
			v.name = f->read_str();
			v.value = f->read_str();
			me->meta_data.variables.add(v);
		}
	}
	void write(File *f) override {
		f->write_str(me->meta_data.script_file.str());
		f->write_int(me->meta_data.script_var.num);
		for (int i=0;i<me->meta_data.script_var.num;i++)
			f->write_float(me->meta_data.script_var[i]);

		// new script vars
		f->write_int(me->meta_data.variables.num);
		for (auto &v: me->meta_data.variables) {
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}
};

class ChunkModel : public FileChunk<DataModel, DataModel> {
public:
	ChunkModel() : FileChunk("model") {}
	void define_children() override {
		add_child(new ChunkMeta);
		add_child(new ChunkMaterial);
		add_child(new ChunkTriangleMesh);
		add_child(new ChunkPolygonMesh);
		add_child(new ChunkPhysicalMesh);
		add_child(new ChunkSkeleton);
		add_child(new ChunkAnimation);
		add_child(new ChunkScript);
		add_child(new ChunkEffect);
		add_child(new ChunkOldMeta);
	}
	void read(File *f) override {
	}
	void write(File *f) override {
	}
	void write_subs() override {
		write_sub("meta", me);
		write_sub_parray("material", me->material);
		write_sub_array("triamesh", me->skin.sub(1, -1));
		write_sub("physmesh", me->phys_mesh);
		write_sub("polymesh", me->mesh);
		if (me->bone.num > 0)
			write_sub("skeleton", me);
		if (me->move.num > 0)
			write_sub("animation", me);
		if (!me->meta_data.script_file.is_empty() or me->meta_data.script_var.num > 0)
			write_sub("script", me);
		write_sub_array("effect", me->fx);

		if (me->meta_data.name != "" or me->meta_data.description != "" or me->meta_data.inventary.num > 0)
			write_sub("xxx1", me);
	}
};


class ModelParser : public ChunkedFileParser {
public:
	ModelParser() : ChunkedFileParser(8) {
		_model_parser_tria_mesh_count = 0;
		set_base(new ChunkModel);
	}
	void on_notify() override {}
	void on_unhandled() override {
		ed->set_message("unhandled chunk " + context.str());
	}
	void on_error(const string &message) override {
		ed->error_box(message);
	}
	void on_warn(const string &message) override {}
	void on_info(const string &message) override {}
};

void FormatModel::_load(const Path &filename, DataModel *data, bool deep) {

	char c = read_first_char(filename);
	data->file_time = file_mtime(filename).time;

	if (c == 'b' or c == 't') {
		_load_old(filename, data, deep);
	} else {
		ModelParser p;
		data->material.clear();
		p.read(filename, data);
	}








	if (deep) {

		// import...
		if (data->mesh->polygon.num == 0)
			data->importFromTriangleSkin(1);

		for (ModelMove &m: data->move)
			if (m.type == MOVE_TYPE_VERTEX) {
				for (ModelFrame &f: m.frame)
					f.vertex_dpos = f.skin[1].dpos;
			}

		for (auto *m: data->material) {
			m->make_consistent();

			// test textures
			for (auto &t: m->texture_levels) {
				if ((!t->texture) and (!t->filename.is_empty()))
					warning(format(_("Texture file not loadable: %s"), t->filename));
			}
		}
		for (auto &b: data->bone) {
			try {
				if (!b.model)
					b.model = ModelManager::load(b.model_file);
			} catch(Exception &e) {
				msg_error(e.message());
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data) {
			ed->SetTitle(filename);
			ResetView();
		}*/
	}

	// FIXME
	if ((!data->meta_data.script_file.is_empty()) and (data->meta_data.variables.num == 0)) {
		update_model_script_data(data->meta_data);
		msg_write(data->meta_data.variables.num);
		for (int i=0; i<min(data->meta_data.script_var.num, data->meta_data.variables.num); i++) {
			if (data->meta_data.variables[i].type == "float")
				data->meta_data.variables[i].value = f2s(data->meta_data.script_var[i], 6);
			msg_write(format("  try import var  %s = %s", data->meta_data.variables[i].name, data->meta_data.variables[i].value));
		}
	}



	if (deep)
		data->on_post_action_update();
}

void FormatModel::_save(const Path &filename, DataModel *data) {
	if (DataModelAllowUpdating) {
		/*if (AutoGenerateSkin[1])
			CreateSkin(&Skin[1],&Skin[2],(float)DetailFactor[1]*0.01f);

		if (AutoGenerateSkin[2])
			CreateSkin(&Skin[2],&Skin[3],(float)DetailFactor[2]/(float)DetailFactor[1]);*/

		if (data->meta_data.auto_generate_dists)
			data->generateDetailDists(data->meta_data.detail_dist);

		if (data->meta_data.auto_generate_tensor)
			data->meta_data.inertia_tensor = data->generateInertiaTensor(data->meta_data.mass);



	#ifdef FORCE_UPDATE_NORMALS
		for (int d=1;d<4;d++)
			for (int j=0;j<skin[d].NumVertices;j++)
				skin[d].vertex[j].normal_dirty = true;
	#endif
		data->updateNormals();

		// export...
		data->mesh->export_to_triangle_skin(data->skin[1]);
		for (int d=1;d<4;d++) {
			if (data->skin[d].sub.num != data->material.num) {
				data->skin[d].sub.resize(data->material.num);
			}
		}


	//	PrecreatePhysicalData();
	}


	// so the materials don't get mixed up
//	RemoveUnusedData();

	ModelParser p;
	p.write(filename, data);

//	_save_v11(filename, data);

}

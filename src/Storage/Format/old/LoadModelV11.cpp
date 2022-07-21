/*
 * LoadModelV11.cpp
 *
 *  Created on: Feb 6, 2021
 *      Author: michi
 */

#include "../FormatModel.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/ModelMesh.h"
#include "../../../Data/Model/ModelPolygon.h"
#include "../../../Mode/Model/ModeModel.h"
#include "../../../y/ModelManager.h"
#include "../../../y/components/Animator.h"
#include "../../../lib/os/file.h"
#include "../../../lib/os/filesystem.h"
#include "../../../lib/os/formatter.h"
#include "../../../Edward.h"

vector get_normal_by_index(int index);


int find_other_poly_from_edge(ModelMesh *m, int e, int t) {
	if (m->edge[e].polygon[0] == t)
		return m->edge[e].polygon[1];
	return m->edge[e].polygon[0];
}

void guess_smooth_groups(DataModel *m) {
	auto mesh = m->mesh;

	std::function<void(Set<int>&, Set<int>&, ModelPolygon&,int,int,int&)> add_and_grow_outwards = [&](Set<int> &polys, Set<int> &edges, ModelPolygon &p, int index, int g, int &n_found) {
		if (polys.contains(index))
			return;
		p.smooth_group = g;
		polys.add(index);
		n_found ++;

		for (int k=0; k<p.side.num; k++) {
			int ei = p.side[k].edge;
			edges.add(ei);
			int pp = find_other_poly_from_edge(mesh, ei, index);
			if (pp < 0)
				continue;



			ModelEdge &e = mesh->edge[ei];

			// adjoined triangles
			ModelPolygon &t1 = mesh->polygon[e.polygon[0]];
			ModelPolygon &t2 = mesh->polygon[e.polygon[1]];

			ModelVertex &v1 = mesh->vertex[e.vertex[0]];
			ModelVertex &v2 = mesh->vertex[e.vertex[1]];

			// round?
			e.is_round = false;
			if ((v1.normal_mode == NORMAL_MODE_ANGULAR) or (v2.normal_mode == NORMAL_MODE_ANGULAR))
				e.is_round = (t1.temp_normal * t2.temp_normal > 0.6f);

			if ((v1.normal_mode == NORMAL_MODE_SMOOTH) or (v2.normal_mode == NORMAL_MODE_SMOOTH))
				e.is_round = true;

			if (e.is_round)
				add_and_grow_outwards(polys, edges, mesh->polygon[pp], pp, g, n_found);
		}
	};

	for (auto &p: mesh->polygon)
		p.temp_normal = p.get_normal(mesh->vertex);


	foreachi (auto &p, mesh->polygon, i) {
		if (p.smooth_group >= 0)
			continue;
		int g = randi(50000);
		int n_found = 0;

		Set<int> polys;
		Set<int> edges;

		add_and_grow_outwards(polys, edges, p, i, g, n_found);

		if (n_found == 1)
			p.smooth_group = -1;
	}

	Set<int> groups;
	for (auto &p: mesh->polygon)
		if (p.smooth_group >= 0)
			groups.add(p.smooth_group);
	msg_write(format(" %d smooth groups found", groups.num));
}

BinaryFormatter *load_file_x(const Path &filename, int &version);

void FormatModel::_load_old(const Path &filename, DataModel *data, bool deep) {
	// old format

	int ffv;
	auto f = load_file_x(filename, ffv);

	if (ffv == 10) {
		_load_v10(f, data, deep);
	} else if (ffv == 11) {
		_load_v11(f, data, deep);

		if (os::fs::exists(filename.with(".edit"))) {
			int ffv2;
			auto ff = load_file_x(filename.with(".edit"), ffv2);
			_load_v11_edit(ff, data, deep);
			delete ff;
		}
	} else {
		throw FormatError(format(_("File %s has a wrong (old) file format: %d (expected: %d - %d)!"), filename, ffv, 10, 11));
	}

	delete f;
}


template<class F>
void FormatModel::_load_v10(F *f, DataModel *data, bool deep) {

	Array<vector> skin_vert;

	// Materials
	f->read_comment();
	data->material.resize(f->read_int());
	for (auto &m: data->material) {
		m = new ModelMaterial();
		m->filename = f->read_str();
		m->col.user = f->read_bool();
		if (m->col.user){
			color am, di, sp, em;
			read_color_argb(f, am);
			read_color_argb(f, di);
			read_color_argb(f, sp);
			read_color_argb(f, em);
			float shininess = (float)f->read_int();
			m->col.import(am, di, sp, shininess, em);
		}
		m->alpha.mode = (TransparencyMode)f->read_int();
		m->alpha.source = (nix::Alpha)f->read_int();
		m->alpha.destination = (nix::Alpha)f->read_int();
		m->alpha.factor = (float)f->read_int() * 0.01f;
		m->alpha.zbuffer = f->read_bool();
		int n = f->read_int();
		m->texture_levels.clear();
		for (int t=0;t<n;t++) {
			auto *tl = new ModelMaterial::TextureLevel();
			tl->filename = f->read_str();
			m->texture_levels.add(tl);
		}
	}
	// create subs...
	for (int k=0;k<4;k++){
		data->triangle_mesh[k].sub.resize(data->material.num);
		for (int j=0;j<data->material.num;j++)
			data->triangle_mesh[k].sub[j].num_textures = 1;
	}

// Physical Skin

	// vertices
	f->read_comment();
	data->phys_mesh->vertex.resize(f->read_int());
	for (auto &v: data->phys_mesh->vertex){
		v.bone_index = {max(f->read_int(), 0), 0, 0, 0};
		v.bone_weight = {1,0,0,0};
		f->read_vector(&v.pos);
	}

	// triangles
	data->triangle_mesh[0].sub[0].triangle.resize(f->read_int());
	for (int j=0;j<data->triangle_mesh[0].sub[0].triangle.num;j++){
		data->triangle_mesh[0].sub[0].triangle[j].normal_dirty = true;
		for (int k=0;k<3;k++)
			data->triangle_mesh[0].sub[0].triangle[j].vertex[k] = f->read_int();
	}

	// balls
	data->phys_mesh->ball.resize(f->read_int());
	for (auto &b: data->phys_mesh->ball){
		b.index = f->read_int();
		b.radius = f->read_float();
	}

	// polys
	if (f->read_int() > 0) {
		msg_error("aaaaaah v10 polyhedron...");
		throw Exception("ag");
	}
	/*data->phys_mesh->polyhedron.resize(f->read_int());
	for (auto &p: data->phys_mesh->polyhedron) {
		p.NumFaces = f->read_int();
		for (int k=0;k<p.NumFaces;k++){
			p.Face[k].NumVertices = f->read_int();
			for (int l=0;l<p.Face[k].NumVertices;l++)
				p.Face[k].Index[l] = f->read_int();
		}
	}*/

// Skin[i]
	for (int i=1;i<4;i++){
		f->read_comment();
		int normal_mode_all = f->read_int();
		bool pre_normals = (normal_mode_all & NORMAL_MODE_PRE) > 0;
		normal_mode_all -= (normal_mode_all & NORMAL_MODE_PRE);

		// vertices
		data->triangle_mesh[i].vertex.resize(f->read_int());
		for (int j=0;j<data->triangle_mesh[i].vertex.num;j++){
			data->triangle_mesh[i].vertex[j].bone_index = {f->read_int(), 0, 0, 0};
			data->triangle_mesh[i].vertex[j].bone_index = {1,0,0,0};
			if (data->triangle_mesh[i].vertex[j].bone_index.i < 0)
				data->triangle_mesh[i].vertex[j].bone_index.i = 0;
			f->read_vector(&data->triangle_mesh[i].vertex[j].pos);
			if (normal_mode_all == NORMAL_MODE_PER_VERTEX)
				data->triangle_mesh[i].vertex[j].normal_mode = f->read_byte();
			else
				data->triangle_mesh[i].vertex[j].normal_mode = normal_mode_all;
			data->triangle_mesh[i].vertex[j].normal_dirty = true;
		}

		// skin vertices
		skin_vert.resize(f->read_int());
		for (int j=0;j<skin_vert.num;j++){
			skin_vert[j].x = f->read_float();
			skin_vert[j].y = f->read_float();
		}



		// triangles (subs)
		int num_trias = f->read_int();
		for (int t=0;t<data->material.num;t++)
			data->triangle_mesh[i].sub[t].triangle.resize(f->read_int());
		for (int t=0;t<data->material.num;t++)
			for (int j=0;j<data->triangle_mesh[i].sub[t].triangle.num;j++)
				for (int k=0;k<3;k++){
					data->triangle_mesh[i].sub[t].triangle[j].vertex[k] = f->read_int();
					int svi = f->read_int();
					data->triangle_mesh[i].sub[t].triangle[j].skin_vertex[0][k] = skin_vert[svi];
					int normal_index = (int)f->read_byte();
					//data->triangle_mesh[i].sub[t].triangle[j].normal[k] = get_normal_by_index(normal_index);
					data->triangle_mesh[i].sub[t].triangle[j].normal_dirty = true;
				}
	}

// Skeleton
	f->read_comment();
	data->bone.resize(f->read_int());
	for (auto &b: data->bone){
		f->read_vector(&b.pos);
		b.parent = f->read_int();
		if (b.parent > 32000)
			b.parent = -1;
		if (b.parent >= 0)
			b.pos += data->bone[b.parent].pos;
		b.model_file = f->read_str();
		if (deep)
			b.model = ModelManager::load(b.model_file);
		b.const_pos = false;
		b.is_selected = b.m_old = false;
	}

// Animations
	f->read_comment();
	int num_anims = f->read_int();
	for (int i=0;i<num_anims;i++){
		int anim_index = f->read_int();
		data->move.resize(anim_index + 1);
		ModelMove *m = &data->move[anim_index];
		m->name = f->read_str();
		m->type = (AnimationType)f->read_int();
		m->frame.resize(f->read_int());
		m->frames_per_sec_const = f->read_float();
		m->frames_per_sec_factor = f->read_float();

		// vertex animation
		if (m->type == AnimationType::VERTEX){
			for (int fr=0;fr<m->frame.num;fr++){
				m->frame[fr].duration = 1;
				for (int s=0;s<4;s++){
					m->frame[fr].skin[s].dpos.resize(data->triangle_mesh[s].vertex.num);
					int num_vertices = f->read_int();
					for (int j=0;j<num_vertices;j++){
						int vertex_index = f->read_int();
						f->read_vector(&m->frame[fr].skin[s].dpos[vertex_index]);
					}
				}
			}
		}else if (m->type == AnimationType::SKELETAL){
			Array<bool> VarDeltaPos;
			VarDeltaPos.resize(data->bone.num);
			for (int j=0;j<data->bone.num;j++)
				VarDeltaPos[j] = f->read_bool();
			m->interpolated_quadratic = f->read_bool();
			m->interpolated_loop = f->read_bool();
			for (int fr=0;fr<m->frame.num;fr++){
				m->frame[fr].duration = 1;
				m->frame[fr].skel_dpos.resize(data->bone.num);
				m->frame[fr].skel_ang.resize(data->bone.num);
				for (int j=0;j<data->bone.num;j++){
					f->read_vector(&m->frame[fr].skel_ang[j]);
					if (VarDeltaPos[j])
						f->read_vector(&m->frame[fr].skel_dpos[j]);
				}
			}
		}
	}
	// Effects
	f->read_comment();
	data->fx.resize(f->read_int());
	if (data->fx.num>10000)
		data->fx.clear();
	for (int i=0;i<data->fx.num;i++){
		string fxkind = f->read_str();
		data->fx[i].type = -1;
		if (fxkind == "Script"){
			data->fx[i].type = FX_TYPE_SCRIPT;
			data->fx[i].vertex = f->read_int();
			data->fx[i].file = f->read_str();
			f->read_str();
		}
		if (fxkind == "Light"){
			data->fx[i].type = FX_TYPE_LIGHT;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			for (int j=0;j<3;j++)
				read_color_argb(f,data->fx[i].colors[j]);
		}
		if (fxkind == "Sound"){
			data->fx[i].type = FX_TYPE_SOUND;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			data->fx[i].speed = (float)f->read_int() * 0.01f;
			data->fx[i].file = f->read_str();
		}
		if (fxkind == "ForceField"){
			data->fx[i].type = FX_TYPE_FORCEFIELD;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			data->fx[i].intensity = (float)f->read_int();
			data->fx[i].inv_quad = f->read_bool();
		}
		if (data->fx[i].type<0)
			throw FormatError("unknown effect: " + fxkind);
	}
	// LOD-Distances
	f->read_comment();
	data->meta_data.detail_dist[0]=f->read_float();
	data->meta_data.detail_dist[1]=f->read_float();
	data->meta_data.detail_dist[2]=f->read_float();
	data->meta_data.auto_generate_dists=f->read_bool();
	data->meta_data.detail_factor[1]=f->read_byte();
	data->meta_data.detail_factor[2]=f->read_byte();
	data->meta_data.auto_generate_skin[1]=f->read_bool();
	data->meta_data.auto_generate_skin[2]=f->read_bool();
	// Physics
	f->read_comment();
	data->meta_data.auto_generate_tensor = f->read_bool();
	for (int i=0;i<9;i++)
		data->meta_data.inertia_tensor.e[i] = f->read_float();
	// BG Textures
	/*if (strcmp(f->read_str(),"#")!=0){
		for (int i=0;i<4;i++){
			char sss[512];
			strcpy(sss,f->read_str());
			if (strlen(sss)>0){
				strcpy(BgTextureFile[i],sss);
				if (allow_load)
					BgTexture[i]=NixLoadTexture(sss);
				BgTextureA[i].x=f->read_float();
				BgTextureA[i].y=f->read_float();
				BgTextureA[i].z=f->read_float();
				BgTextureB[i].x=f->read_float();
				BgTextureB[i].y=f->read_float();
				BgTextureB[i].z=f->read_float();
			}
		}
	}*/

	// correction
	/*skin=&Skin[1];
	SetNormalMode(NormalModeAngular,true);
	skin=&Skin[2];
	SetNormalMode(NormalModeAngular,true);
	skin=&Skin[3];
	SetNormalMode(NormalModeAngular,true);
	AlphaZBuffer=(TransparencyMode!=TransparencyMode::FUNCTIONS)and(TransparencyMode!=TransparencyMode::FACTOR);*/

	guess_smooth_groups(data);
}


template<class F>
void FormatModel::_load_v11(F *f, DataModel *data, bool deep) {


	Array<vector> skin_vert;

	// General
	vector tv;
	f->read_comment();
	// bounding box
	f->read_vector(&tv);
	f->read_vector(&tv);
	// skins
	f->read_int();
	// reserved
	f->read_int();
	f->read_int();
	f->read_int();
	//

	// Materials
	f->read_comment();
	data->material.resize(f->read_int());
	for (auto &m: data->material){
		m = new ModelMaterial();
		m->filename = f->read_str();
		m->col.user = f->read_bool();
		color am, di, sp, em;
		read_color_argb(f, am);
		read_color_argb(f, di);
		read_color_argb(f, sp);
		read_color_argb(f, em);
		float shininess = (float)f->read_int();
		m->col.import(em, di, sp, shininess, em);
		m->alpha.mode = (TransparencyMode)f->read_int();
		m->alpha.source = (nix::Alpha)f->read_int();
		m->alpha.destination = (nix::Alpha)f->read_int();
		m->alpha.factor = (float)f->read_int() * 0.01f;
		m->alpha.zbuffer = f->read_bool();
		int n = f->read_int();
		m->texture_levels.clear();
		for (int t=0;t<n;t++) {
			auto tl = new ModelMaterial::TextureLevel();
			tl->filename = f->read_str();
			m->texture_levels.add(tl);
		}
	}
	// create subs...
	for (int k=0;k<4;k++){
		data->triangle_mesh[k].sub.resize(data->material.num);
		for (int j=0;j<data->material.num;j++)
			data->triangle_mesh[k].sub[j].num_textures = data->material[j]->texture_levels.num;
	}

// Physical Skin

	// vertices
	f->read_comment();
	data->phys_mesh->vertex.resize(f->read_int());
	for (int j=0;j<data->phys_mesh->vertex.num;j++) {
		data->phys_mesh->vertex[j].bone_index = {f->read_int(), 0, 0, 0};
		data->phys_mesh->vertex[j].bone_weight = {1, 0, 0, 0};
	}
	for (int j=0;j<data->phys_mesh->vertex.num;j++)
		f->read_vector(&data->phys_mesh->vertex[j].pos);

	msg_write("phys vert: " + i2s(data->phys_mesh->vertex.num));

	// triangles
	f->read_int();

	// balls
	data->phys_mesh->ball.resize(f->read_int());
	for (auto &b: data->phys_mesh->ball){
		b.index = f->read_int();
		b.radius = f->read_float();
	}

	// polys
	int num_polys = f->read_int();
	for (int i=0; i<num_polys; i++){
		msg_write("POLYHEDRON! " + format("%d/%d", i, num_polys));

		Array<Array<int>> vv;
		int NumFaces = f->read_int();
		msg_write("  faces: " + i2s(NumFaces));
		for (int k=0;k<NumFaces;k++){
			int nv = f->read_int();
			Array<int> vertex;
			for (int l=0;l<nv;l++) {
				vertex.add(f->read_int());
			}
			msg_write(ia2s(vertex));
			vv.add(vertex);
			// plane xyzd
			f->read_float();
			f->read_float();
			f->read_float();
			f->read_float();
		}
		int NumSVertices = f->read_int();
		for (int k=0;k<NumSVertices;k++)
			f->read_int();
		int NumEdges = f->read_int();
		for (int k=0;k<NumEdges;k++){
			f->read_int();
			f->read_int();
		}
		// topology
		for (int k=0;k<NumFaces;k++)
			for (int l=0;l<NumFaces;l++)
				f->read_int();
		for (int k=0;k<NumEdges;k++)
			for (int l=0;l<NumFaces;l++)
				f->read_bool();

		Array<int> relink;
		for (auto &_vv: vv) {
			for (int &v: _vv) {
				for (int k=0; k<relink.num; k+=2)
					if (relink[k] == v) {
						v = relink[k+1];
						break;
					}
				if (data->phys_mesh->vertex[v].ref_count > 0) {
					//msg_write("clone vertex");
					int nv = data->phys_mesh->vertex.num;
					relink.append({v, nv});
					msg_write(format("  relink %d  %d", v, nv));
					data->phys_mesh->add_vertex(data->phys_mesh->vertex[v].pos, {0,0,0,0}, {1,0,0,0}, 0);
					v = nv;
				}
			}
		}
		for (auto &_vv: vv) {
			Array<vector> sv;
			sv.resize(_vv.num * MATERIAL_MAX_TEXTURES);//data->material[0]->texture_levels.num);
			//msg_write(ia2s(_vv));
			try{
				msg_write(" + poly " + ia2s(_vv));
				data->phys_mesh->_add_polygon(_vv, 0, sv);
			}catch(GeometryException &e) {
				msg_error(e.message);
			}
		}
	}

// Skin[i]
	for (int i=1;i<4;i++){

		// vertices
		f->read_comment();
		data->triangle_mesh[i].vertex.resize(f->read_int());
		for (int j=0;j<data->triangle_mesh[i].vertex.num;j++)
			f->read_vector(&data->triangle_mesh[i].vertex[j].pos);
		for (int j=0;j<data->triangle_mesh[i].vertex.num;j++) {
			data->triangle_mesh[i].vertex[j].bone_index = {f->read_int(), 0,0,0};
			data->triangle_mesh[i].vertex[j].bone_weight = {1,0,0,0};
		}
		for (int j=0;j<data->triangle_mesh[i].vertex.num;j++)
			data->triangle_mesh[i].vertex[j].normal_dirty = false;//true;

		// skin vertices
		skin_vert.resize(f->read_int());
		for (int j=0;j<skin_vert.num;j++){
			skin_vert[j].x = f->read_float();
			skin_vert[j].y = f->read_float();
		}



		// triangles (subs)
		for (int m=0;m<data->material.num;m++){
			data->triangle_mesh[i].sub[m].triangle.resize(f->read_int());
			// vertex
			for (int j=0;j<data->triangle_mesh[i].sub[m].triangle.num;j++)
				for (int k=0;k<3;k++)
					data->triangle_mesh[i].sub[m].triangle[j].vertex[k] = f->read_int();
			// skin vertex
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<data->triangle_mesh[i].sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						int svi = f->read_int();
						data->triangle_mesh[i].sub[m].triangle[j].skin_vertex[tl][k] = skin_vert[svi];
					}
			// normals
			for (int j=0;j<data->triangle_mesh[i].sub[m].triangle.num;j++){
				for (int k=0;k<3;k++){
					int normal_index = (int)(unsigned short)f->read_word();
					data->triangle_mesh[i].sub[m].triangle[j].normal[k] = get_normal_by_index(normal_index);
				}
				data->triangle_mesh[i].sub[m].triangle[j].normal_dirty = false;
			}
			f->read_int();
		}
		f->read_int();
	}

// Skeleton
	f->read_comment();
	data->bone.resize(f->read_int());
	for (ModelBone &b: data->bone){
		f->read_vector(&b.pos);
		b.parent = f->read_int();
		if ((b.parent < 0) || (b.parent >= data->bone.num))
			b.parent = -1;
		if (b.parent >= 0)
			b.pos += data->bone[b.parent].pos;
		b.model_file = f->read_str();
		try{
			if (deep)
			b.model = ModelManager::load(b.model_file);
		} catch(Exception &e) {
			msg_error(e.message());
		}
		b.const_pos = false;
		b.is_selected = b.m_old = false;
	}

// Animations
	f->read_comment();
	data->move.resize(f->read_int());
	int num_anims = f->read_int();
	f->read_int();
	f->read_int();
	for (int i=0;i<num_anims;i++){
		int anim_index = f->read_int();
		data->move.resize(anim_index + 1);
		ModelMove *m = &data->move[anim_index];
		m->name = f->read_str();
		int tt = f->read_int();
		m->type = (AnimationType)(tt & 0x7f);
		bool rubber_timing = (tt & 128);
		m->frame.resize(f->read_int());
		m->frames_per_sec_const = f->read_float();
		m->frames_per_sec_factor = f->read_float();

		// vertex animation
		if (m->type == AnimationType::VERTEX){
			for (ModelFrame &fr: m->frame){
				fr.duration = 1;
				if (rubber_timing)
					fr.duration = f->read_float();
				for (int s=0;s<4;s++){
					fr.skin[s].dpos.resize(data->triangle_mesh[s].vertex.num);
					int num_vertices = f->read_int();
					for (int j=0;j<num_vertices;j++){
						int vertex_index = f->read_int();
						f->read_vector(&fr.skin[s].dpos[vertex_index]);
					}
				}
			}
		}else if (m->type == AnimationType::SKELETAL){
			Array<bool> VarDeltaPos;
			VarDeltaPos.resize(data->bone.num);
			for (int j=0;j<data->bone.num;j++)
				VarDeltaPos[j] = f->read_bool();
			m->interpolated_quadratic = f->read_bool();
			m->interpolated_loop = f->read_bool();
			for (ModelFrame &fr: m->frame){
				fr.duration = 1;
				if (rubber_timing)
					fr.duration = f->read_float();
				fr.skel_dpos.resize(data->bone.num);
				fr.skel_ang.resize(data->bone.num);
				for (int j=0;j<data->bone.num;j++){
					f->read_vector(&fr.skel_ang[j]);
					if (VarDeltaPos[j])
						f->read_vector(&fr.skel_dpos[j]);
				}
			}
		}else{
			throw FormatError("unknown animation type: " + i2s((int)m->type));
		}
	}
	// Effects
	f->read_comment();
	data->fx.resize(f->read_int());
	if (data->fx.num>10000)
		data->fx.clear();
	for (int i=0;i<data->fx.num;i++){
		string fxkind = f->read_str();
		data->fx[i].type=-1;
		if (fxkind == "Script"){
			data->fx[i].type = FX_TYPE_SCRIPT;
			data->fx[i].vertex = f->read_int();
			data->fx[i].file = f->read_str();
			f->read_str();
		}else if (fxkind == "Light"){
			data->fx[i].type = FX_TYPE_LIGHT;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			for (int j=0;j<3;j++)
				read_color_argb(f,data->fx[i].colors[j]);
		}else if (fxkind == "Sound"){
			data->fx[i].type = FX_TYPE_SOUND;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			data->fx[i].speed = (float)f->read_int() * 0.01f;
			data->fx[i].file = f->read_str();
		}else if (fxkind == "ForceField"){
			data->fx[i].type = FX_TYPE_FORCEFIELD;
			data->fx[i].vertex = f->read_int();
			data->fx[i].size = (float)f->read_int();
			data->fx[i].intensity = (float)f->read_int();
			data->fx[i].inv_quad = f->read_bool();
		}else{
			throw FormatError("unknown effect: " + fxkind);
		}
	}

// properties
	// Physics
	f->read_comment();
	data->meta_data.mass = f->read_float();
	for (int i=0;i<9;i++)
		data->meta_data.inertia_tensor.e[i] = f->read_float();
	data->meta_data.active_physics = f->read_bool();
	data->meta_data.passive_physics = f->read_bool();
	f->read_float(); // radius

	// LOD-Distances
	f->read_comment();
	data->meta_data.detail_dist[0] = f->read_float();
	data->meta_data.detail_dist[1] = f->read_float();
	data->meta_data.detail_dist[2] = f->read_float();

// object data
	// Object Data
	f->read_comment();
	data->meta_data.name = f->read_str();
	data->meta_data.description = f->read_str();

	// Inventary
	f->read_comment();
	data->meta_data.inventary.resize(f->read_int());
	for (int i=0;i<data->meta_data.inventary.num;i++){
		data->meta_data.inventary[i] = f->read_str();
		f->read_int();
	}

	// Script
	f->read_comment();
	data->meta_data.script_file = f->read_str();
	data->meta_data.script_var.resize(f->read_int());
	for (int i=0;i<data->meta_data.script_var.num;i++)
		data->meta_data.script_var[i] = f->read_float();

	_load_v11_edit(f, data, deep);

	guess_smooth_groups(data);
}

template<class F>
void FormatModel::_load_v11_edit(F *f, DataModel *data, bool deep) {

// optional data / additional data for editing
	while (true) {
		string s = f->read_str();
		msg_write("opt:" + s);
		if (s == "// Editor"){
			data->meta_data.auto_generate_tensor = f->read_bool();
			data->meta_data.auto_generate_dists = f->read_bool();
			data->meta_data.auto_generate_skin[1] = f->read_bool();
			data->meta_data.auto_generate_skin[2] = f->read_bool();
			data->meta_data.detail_factor[1] = f->read_int();
			data->meta_data.detail_factor[2] = f->read_int();
		}else if (s == "// Normals"){
			for (int i=1;i<4;i++){
				ModelTriangleMesh *s = &data->triangle_mesh[i];
				int normal_mode_all = f->read_int();
				if (normal_mode_all == NORMAL_MODE_PER_VERTEX){
					for (ModelVertex &v: s->vertex)
						v.normal_mode = f->read_int();
				}else{
					for (ModelVertex &v: s->vertex)
						v.normal_mode = normal_mode_all;
				}
			}
		}else if (s == "// Polygons"){
			//data->begin_action_group("LoadPolygonData");
			foreachi(ModelVertex &v, data->triangle_mesh[1].vertex, i)
				data->addVertex(v.pos, v.bone_index, v.bone_weight, v.normal_mode);
			int ns = f->read_int();
			for (int i=0;i<ns;i++){
				int nv = f->read_int();
				for (int j=0;j<nv;j++){
					ModelPolygon t;
					t.is_selected = false;
					t.triangulation_dirty = true;
					int n = f->read_int();
					t.material = f->read_int();
					t.side.resize(n);
					for (int k=0;k<n;k++){
						t.side[k].vertex = f->read_int();
						for (int l=0;l<data->material[t.material]->texture_levels.num;l++){
							t.side[k].skin_vertex[l].x = f->read_float();
							t.side[k].skin_vertex[l].y = f->read_float();
						}
					}
					t.normal_dirty = true;
					data->mesh->polygon.add(t);
				}
				f->read_bool();
				f->read_bool();
				f->read_int();
			}
			//data->end_action_group();
			data->mesh->build_topology();
		}else if (s == "// Cylinders"){
			int n = f->read_int();
			for (int i=0; i<n; i++){
				ModelCylinder c;
				c.index[0] = f->read_int();
				c.index[1] = f->read_int();
				c.radius = f->read_float();
				c.round = f->read_bool();
				data->phys_mesh->cylinder.add(c);
			}
		}else if (s == "// Script Vars"){
			int n = f->read_int();
			for (int i=0; i<n; i++){
				ModelScriptVariable v;
				v.name = f->read_str();
				v.value = f->read_str();
				data->meta_data.variables.add(v);
			}
		}else if (s == "#"){
			break;
		}else{
			warning("unknown optional: " + s);
			break;
		}
	}
}




void FormatModel::_save_v11(const Path &filename, DataModel *data) {

	auto f = new BinaryFormatter(os::fs::open(filename, "wb"));
	//f->WriteFileFormatVersion(true, 11);//FFVBinary, 11);
	f->write("b");
	f->write_word(11);
	//f->float_decimals = 5;




//	f->write_comment("// LOD-Distances");
//	f->write_float(data->meta_data.detail_dist[0]);
//	f->write_float(data->meta_data.detail_dist[1]);
//	f->write_float(data->meta_data.detail_dist[2]);


	// additional data for editing
	f->write_str("// Editor");
	f->write_bool(data->meta_data.auto_generate_tensor);
	f->write_bool(data->meta_data.auto_generate_dists);
	f->write_bool(data->meta_data.auto_generate_skin[1]);
	f->write_bool(data->meta_data.auto_generate_skin[2]);
	f->write_int(data->meta_data.detail_factor[1]);
	f->write_int(data->meta_data.detail_factor[2]);

	delete f;
}

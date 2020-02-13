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
#include "../../x/model_manager.h"

FormatModel::FormatModel() : TypedFormat<DataModel>(FD_MODEL, "model", _("Model"), Flag::CANONICAL_READ_WRITE) {
}


void update_model_script_data(DataModel::MetaData &m);

bool DataModelAllowUpdating = true;


int get_normal_index(vector &n)
{
	int nxy, nz;
	if ((n.x == 0) and (n.y == 0)){
		nxy = 0;
		nz = (n.z < 0) ? 255 : 0;
	}else{
		nxy = (int)(atan2(n.y, n.x) / pi / 2 * 255.0f) & 255;
		nz = (int)(acos(n.z) / pi * 255.0f) & 255;
	}
	return nxy + 256 * nz;
}

vector get_normal_by_index(int index)
{
	float wz = (float)(index >> 8) * pi / 255.0f;
	float wxy = (float)(index & 255) * 2 * pi / 255.0f;
	float swz = sin(wz);
	if (swz < 0)
		swz = - swz;
	float cwz = cos(wz);
	return vector( cos(wxy) * swz, sin(wxy) * swz, cwz);
}

void FormatModel::_load_v10(File *f, DataModel *data, bool deep) {

	Array<vector> skin_vert;

	// Materials
	f->read_comment();
	data->material.resize(f->read_int());
	for (auto &m: data->material) {
		m = new ModelMaterial();
		m->filename = f->read_str();
		m->col.user = f->read_bool();
		if (m->col.user){
			read_color_argb(f, m->col.ambient);
			read_color_argb(f, m->col.diffuse);
			read_color_argb(f, m->col.specular);
			read_color_argb(f, m->col.emission);
			m->col.shininess = (float)f->read_int();
		}
		m->alpha.mode = f->read_int();
		m->alpha.user = (m->alpha.mode != TransparencyModeDefault);
		m->alpha.source = f->read_int();
		m->alpha.destination = f->read_int();
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
		data->skin[k].sub.resize(data->material.num);
		for (int j=0;j<data->material.num;j++)
			data->skin[k].sub[j].num_textures = 1;
	}

// Physical Skin

	// vertices
	f->read_comment();
	data->skin[0].vertex.resize(f->read_int());
	for (int j=0;j<data->skin[0].vertex.num;j++){
		data->skin[0].vertex[j].bone_index = f->read_int();
		if (data->skin[0].vertex[j].bone_index < 0)
			data->skin[0].vertex[j].bone_index = 0;
		f->read_vector(&data->skin[0].vertex[j].pos);
	}

	// triangles
	data->skin[0].sub[0].triangle.resize(f->read_int());
	for (int j=0;j<data->skin[0].sub[0].triangle.num;j++){
		data->skin[0].sub[0].triangle[j].normal_dirty = true;
		for (int k=0;k<3;k++)
			data->skin[0].sub[0].triangle[j].vertex[k] = f->read_int();
	}

	// balls
	data->phys_mesh->ball.resize(f->read_int());
	for (auto &b: data->phys_mesh->ball){
		b.index = f->read_int();
		b.radius = f->read_float();
	}

	// polys
	data->phys_mesh->polyhedron.resize(f->read_int());
	for (auto &p: data->phys_mesh->polyhedron) {
		p.NumFaces = f->read_int();
		for (int k=0;k<p.NumFaces;k++){
			p.Face[k].NumVertices = f->read_int();
			for (int l=0;l<p.Face[k].NumVertices;l++)
				p.Face[k].Index[l] = f->read_int();
		}
	}

// Skin[i]
	for (int i=1;i<4;i++){
		f->read_comment();
		int normal_mode_all = f->read_int();
		bool pre_normals = (normal_mode_all & NORMAL_MODE_PRE) > 0;
		normal_mode_all -= (normal_mode_all & NORMAL_MODE_PRE);

		// vertices
		data->skin[i].vertex.resize(f->read_int());
		for (int j=0;j<data->skin[i].vertex.num;j++){
			data->skin[i].vertex[j].bone_index = f->read_int();
			if (data->skin[i].vertex[j].bone_index < 0)
				data->skin[i].vertex[j].bone_index = 0;
			f->read_vector(&data->skin[i].vertex[j].pos);
			if (normal_mode_all == NORMAL_MODE_PER_VERTEX)
				data->skin[i].vertex[j].normal_mode = f->read_byte();
			else
				data->skin[i].vertex[j].normal_mode = normal_mode_all;
			data->skin[i].vertex[j].normal_dirty = true;
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
			data->skin[i].sub[t].triangle.resize(f->read_int());
		for (int t=0;t<data->material.num;t++)
			for (int j=0;j<data->skin[i].sub[t].triangle.num;j++)
				for (int k=0;k<3;k++){
					data->skin[i].sub[t].triangle[j].vertex[k] = f->read_int();
					int svi = f->read_int();
					data->skin[i].sub[t].triangle[j].skin_vertex[0][k] = skin_vert[svi];
					data->skin[i].sub[t].triangle[j].normal_index[k] = (int)f->read_byte();
					data->skin[i].sub[t].triangle[j].normal_dirty = true;
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
			b.model = LoadModel(b.model_file);
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
		m->type = f->read_int();
		m->frame.resize(f->read_int());
		m->frames_per_sec_const = f->read_float();
		m->frames_per_sec_factor = f->read_float();

		// vertex animation
		if (m->type == MOVE_TYPE_VERTEX){
			for (int fr=0;fr<m->frame.num;fr++){
				m->frame[fr].duration = 1;
				for (int s=0;s<4;s++){
					m->frame[fr].skin[s].dpos.resize(data->skin[s].vertex.num);
					int num_vertices = f->read_int();
					for (int j=0;j<num_vertices;j++){
						int vertex_index = f->read_int();
						f->read_vector(&m->frame[fr].skin[s].dpos[vertex_index]);
					}
				}
			}
		}else if (m->type == MOVE_TYPE_SKELETAL){
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
	AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)and(TransparencyMode!=TransparencyModeFactor);*/
}


void FormatModel::_load_v11(File *f, DataModel *data, bool deep) {


	Array<vector> skin_vert;

	// General
	vector tv;
	f->read_comment();
	// bounding box
	f->read_vector(&data->_min);
	f->read_vector(&data->_max);
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
		read_color_argb(f, m->col.ambient);
		read_color_argb(f, m->col.diffuse);
		read_color_argb(f, m->col.specular);
		read_color_argb(f, m->col.emission);
		m->col.shininess = (float)f->read_int();
		m->alpha.mode = f->read_int();
		m->alpha.user = (m->alpha.mode != TransparencyModeDefault);
		m->alpha.source = f->read_int();
		m->alpha.destination = f->read_int();
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
		data->skin[k].sub.resize(data->material.num);
		for (int j=0;j<data->material.num;j++)
			data->skin[k].sub[j].num_textures = data->material[j]->texture_levels.num;
	}

// Physical Skin

	// vertices
	f->read_comment();
	data->skin[0].vertex.resize(f->read_int());
	for (int j=0;j<data->skin[0].vertex.num;j++)
		data->skin[0].vertex[j].bone_index = f->read_int();
	for (int j=0;j<data->skin[0].vertex.num;j++)
		f->read_vector(&data->skin[0].vertex[j].pos);

	// triangles
	f->read_int();

	// balls
	data->phys_mesh->ball.resize(f->read_int());
	for (auto &b: data->phys_mesh->ball){
		b.index = f->read_int();
		b.radius = f->read_float();
	}

	// polys
	data->phys_mesh->polyhedron.resize(f->read_int());
	for (auto &p: data->phys_mesh->polyhedron){
		p.NumFaces = f->read_int();
		for (int k=0;k<p.NumFaces;k++){
			p.Face[k].NumVertices = f->read_int();
			for (int l=0;l<p.Face[k].NumVertices;l++)
				p.Face[k].Index[l] = f->read_int();
			p.Face[k].Plane.n.x = f->read_float();
			p.Face[k].Plane.n.y = f->read_float();
			p.Face[k].Plane.n.z = f->read_float();
			p.Face[k].Plane.d = f->read_float();
		}
		p.NumSVertices = f->read_int();
		for (int k=0;k<p.NumSVertices;k++)
			p.SIndex[k] = f->read_int();
		p.NumEdges = f->read_int();
		for (int k=0;k<p.NumEdges;k++){
			p.EdgeIndex[k*2 + 0] = f->read_int();
			p.EdgeIndex[k*2 + 1] = f->read_int();
		}
		// topology
		for (int k=0;k<p.NumFaces;k++)
			for (int l=0;l<p.NumFaces;l++)
				p.FacesJoiningEdge[k * p.NumFaces + l] = f->read_int();
		for (int k=0;k<p.NumEdges;k++)
			for (int l=0;l<p.NumFaces;l++)
				p.EdgeOnFace[k * p.NumFaces + l] = f->read_bool();
	}

// Skin[i]
	for (int i=1;i<4;i++){

		// vertices
		f->read_comment();
		data->skin[i].vertex.resize(f->read_int());
		for (int j=0;j<data->skin[i].vertex.num;j++)
			f->read_vector(&data->skin[i].vertex[j].pos);
		for (int j=0;j<data->skin[i].vertex.num;j++)
			data->skin[i].vertex[j].bone_index = f->read_int();
		for (int j=0;j<data->skin[i].vertex.num;j++)
			data->skin[i].vertex[j].normal_dirty = false;//true;

		// skin vertices
		skin_vert.resize(f->read_int());
		for (int j=0;j<skin_vert.num;j++){
			skin_vert[j].x = f->read_float();
			skin_vert[j].y = f->read_float();
		}



		// triangles (subs)
		for (int m=0;m<data->material.num;m++){
			data->skin[i].sub[m].triangle.resize(f->read_int());
			// vertex
			for (int j=0;j<data->skin[i].sub[m].triangle.num;j++)
				for (int k=0;k<3;k++)
					data->skin[i].sub[m].triangle[j].vertex[k] = f->read_int();
			// skin vertex
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<data->skin[i].sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						int svi = f->read_int();
						data->skin[i].sub[m].triangle[j].skin_vertex[tl][k] = skin_vert[svi];
					}
			// normals
			for (int j=0;j<data->skin[i].sub[m].triangle.num;j++){
				for (int k=0;k<3;k++){
					data->skin[i].sub[m].triangle[j].normal_index[k] = (int)(unsigned short)f->read_word();
					data->skin[i].sub[m].triangle[j].normal[k] = get_normal_by_index(data->skin[i].sub[m].triangle[j].normal_index[k]);
				}
				data->skin[i].sub[m].triangle[j].normal_dirty = false;
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
		if (deep)
			b.model = LoadModel(b.model_file);
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
		m->type = f->read_int();
		bool rubber_timing = (m->type & 128);
		m->type = m->type & 0x7f;
		m->frame.resize(f->read_int());
		m->frames_per_sec_const = f->read_float();
		m->frames_per_sec_factor = f->read_float();

		// vertex animation
		if (m->type == MOVE_TYPE_VERTEX){
			for (ModelFrame &fr: m->frame){
				fr.duration = 1;
				if (rubber_timing)
					fr.duration = f->read_float();
				for (int s=0;s<4;s++){
					fr.skin[s].dpos.resize(data->skin[s].vertex.num);
					int num_vertices = f->read_int();
					for (int j=0;j<num_vertices;j++){
						int vertex_index = f->read_int();
						f->read_vector(&fr.skin[s].dpos[vertex_index]);
					}
				}
			}
		}else if (m->type == MOVE_TYPE_SKELETAL){
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
			throw FormatError("unknown animation type: " + i2s(m->type));
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
	data->radius = f->read_float();

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



// optional data / additional data for editing
	while (true){
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
				ModelSkin *s = &data->skin[i];
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
			foreachi(ModelVertex &v, data->skin[1].vertex, i)
				data->addVertex(v.pos, v.bone_index, v.normal_mode);
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

void FormatModel::_load(const string &filename, DataModel *data, bool deep) {

	int ffv;


	File *f = FileOpenText(filename);
	data->file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();

	if (ffv<0){
		throw FormatError(_("File format unreadable!"));
	}else if (ffv==10){ // old format
		_load_v10(f, data, deep);
	}else if (ffv==11){ // new format
		_load_v11(f, data, deep);
	}else{
		throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 10, 10));
	}

	delete(f);








	if (deep){

		// import...
		if (data->mesh->polygon.num == 0)
			data->importFromTriangleSkin(1);

		for (ModelMove &m: data->move)
			if (m.type == MOVE_TYPE_VERTEX){
				for (ModelFrame &f: m.frame)
					f.vertex_dpos = f.skin[1].dpos;
			}

		for (auto *m: data->material){
			m->makeConsistent();

			// test textures
			for (auto &t: m->texture_levels){
				if ((!t->texture) and (t->filename.num > 0))
					warning(format(_("Texture file not loadable: %s"), t->filename.c_str()));
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data){
			ed->SetTitle(filename);
			ResetView();
		}*/
	}

	// FIXME
	if ((data->meta_data.script_file.num > 0) and (data->meta_data.variables.num == 0)){
		update_model_script_data(data->meta_data);
		msg_write(data->meta_data.variables.num);
		for (int i=0; i<min(data->meta_data.script_var.num, data->meta_data.variables.num); i++){
			if (data->meta_data.variables[i].type == "float")
				data->meta_data.variables[i].value = f2s(data->meta_data.script_var[i], 6);
			msg_write(data->meta_data.variables[i].name);
			msg_write(data->meta_data.variables[i].value);
		}
	}



	if (deep)
		data->on_post_action_update();
}

void FormatModel::_save(const string &filename, DataModel *data) {
	if (DataModelAllowUpdating){
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
		data->exportToTriangleSkin(1);
		for (int d=1;d<4;d++){
			if (data->skin[d].sub.num != data->material.num){
				data->skin[d].sub.resize(data->material.num);
			}
		}


	//	PrecreatePhysicalData();

		data->getBoundingBox(data->_min, data->_max);
		data->radius = data->getRadius() * 1.1f;
	}


	// so the materials don't get mixed up
//	RemoveUnusedData();


	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 11);//FFVBinary, 11);
	f->float_decimals = 5;

// general
	f->write_comment("// General");
	f->write_vector(&data->_min);
	f->write_vector(&data->_max);
	f->write_int(3); // skins...
	f->write_int(0); // reserved
	f->write_int(0);
	f->write_int(0);

// materials
	f->write_comment("// Materials");
	f->write_int(data->material.num);
	for (ModelMaterial *m: data->material){
		f->write_str(m->filename);
		f->write_bool(m->col.user);
		write_color_argb(f, m->col.ambient);
		write_color_argb(f, m->col.diffuse);
		write_color_argb(f, m->col.specular);
		write_color_argb(f, m->col.emission);
		f->write_int(m->col.shininess);
		f->write_int(m->alpha.user ? m->alpha.mode : TransparencyModeDefault);
		f->write_int(m->alpha.source);
		f->write_int(m->alpha.destination);
		f->write_int(m->alpha.factor * 100.0f);
		f->write_bool(m->alpha.zbuffer);
		f->write_int(m->texture_levels.num);
		for (int t=0;t<m->texture_levels.num;t++)
			f->write_str(m->texture_levels[t]->filename);
	}

// physical skin
	f->write_comment("// Physical Skin");

	// vertices
	f->write_int(data->skin[0].vertex.num);
	for (int j=0;j<data->skin[0].vertex.num;j++)
		f->write_int(data->skin[0].vertex[j].bone_index);
	for (int j=0;j<data->skin[0].vertex.num;j++)
		f->write_vector(&data->skin[0].vertex[j].pos);

	// triangles
	f->write_int(0);
	/*for (int j=0;j<Skin[0].NumTriangles;j++)
		for (int k=0;k<3;k++)
			f->write_int(Skin[0].Triangle[j].Index[k]);*/

	// balls
	f->write_int(data->phys_mesh->ball.num);
	for (auto &b: data->phys_mesh->ball){
		f->write_int(b.index);
		f->write_float(b.radius);
	}

	f->write_int(data->phys_mesh->polyhedron.num);
	for (auto &p: data->phys_mesh->polyhedron) {
		f->write_int(p.NumFaces);
		for (int k=0;k<p.NumFaces;k++){
			f->write_int(p.Face[k].NumVertices);
			for (int l=0;l<p.Face[k].NumVertices;l++)
				f->write_int(p.Face[k].Index[l]);
			f->write_float(p.Face[k].Plane.n.x);
			f->write_float(p.Face[k].Plane.n.y);
			f->write_float(p.Face[k].Plane.n.z);
			f->write_float(p.Face[k].Plane.d);
		}
		f->write_int(p.NumSVertices);
		for (int k=0;k<p.NumSVertices;k++)
			f->write_int(p.SIndex[k]);
		f->write_int(p.NumEdges);
		for (int k=0;k<p.NumEdges;k++){
			f->write_int(p.EdgeIndex[k*2 + 0]);
			f->write_int(p.EdgeIndex[k*2 + 1]);
		}
		// topology
		for (int k=0;k<p.NumFaces;k++)
			for (int l=0;l<p.NumFaces;l++)
				f->write_int(p.FacesJoiningEdge[k * p.NumFaces + l]);
		for (int k=0;k<p.NumEdges;k++)
			for (int l=0;l<p.NumFaces;l++)
				f->write_bool(p.EdgeOnFace[k * p.NumFaces + l]);
	}

// skin
	for (int i=1;i<4;i++){
		ModelSkin *s = &data->skin[i];
		f->write_comment(format("// Skin[%d]",i));

		// verices
		f->write_int(s->vertex.num);
		for (ModelVertex &v: s->vertex)
			f->write_vector(&v.pos);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.bone_index);

		// skin vertices
		int num_skin_v = 0;
		for (int m=0;m<data->material.num;m++)
			num_skin_v += s->sub[m].triangle.num * data->material[m]->texture_levels.num * 3;
		f->write_int(num_skin_v);
		for (int m=0;m<data->material.num;m++)
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<s->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].x);
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<data->material.num;m++){
			ModelSubSkin *sub = &s->sub[m];

			// triangles
			f->write_int(sub->triangle.num);

			// vertex index
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++)
					f->write_int(sub->triangle[j].vertex[k]);

			// skin index
			for (int tl=0;tl<data->material[m]->texture_levels.num;tl++)
				for (int j=0;j<sub->triangle.num;j++)
					for (int k=0;k<3;k++)
						f->write_int(svi ++);

			// normal
			for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++){
					if (DataModelAllowUpdating)
						sub->triangle[j].normal_index[k] = get_normal_index(sub->triangle[j].normal[k]);
					f->write_word(sub->triangle[j].normal_index[k]);
				}
			f->write_int(0);
		}

		f->write_int(0);
	}

// skeleton
	f->write_comment("// Skeleton");
	f->write_int(data->bone.num);
	for (ModelBone &b: data->bone){
		if (b.parent >= 0){
			vector dpos = b.pos - data->bone[b.parent].pos;
			f->write_vector(&dpos);
		}else
			f->write_vector(&b.pos);
		f->write_int(b.parent);
		f->write_str(b.model_file);
	}

// animations
	f->write_comment("// Animations");
	if ((data->move.num == 1) and (data->move[0].frame.num == 0)){
		f->write_int(0);
	}else
		f->write_int(data->move.num);
	int n_moves = 0;
	int n_frames_vert = 0;
	int n_frames_skel = 0;
	for (int i=0;i<data->move.num;i++)
		if (data->move[i].frame.num > 0){
			n_moves ++;
			if (data->move[i].type == MOVE_TYPE_VERTEX)	n_frames_vert += data->move[i].frame.num;
			if (data->move[i].type == MOVE_TYPE_SKELETAL)	n_frames_skel += data->move[i].frame.num;
		}
	f->write_int(n_moves);
	f->write_int(n_frames_vert);
	f->write_int(n_frames_skel);
	for (int i=0;i<data->move.num;i++)
		if (data->move[i].frame.num > 0){
			ModelMove *m = &data->move[i];
			bool rubber_timing = m->needsRubberTiming();
			f->write_int(i);
			f->write_str(m->name);
			f->write_int(m->type + (rubber_timing ? 128 : 0));
			f->write_int(m->frame.num);
			f->write_float(m->frames_per_sec_const);
			f->write_float(m->frames_per_sec_factor);

			// vertex animation
			if (m->type == MOVE_TYPE_VERTEX){
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->write_float(fr.duration);
					for (int s=0;s<4;s++){
						// compress (only write != 0)
						int num_vertices = 0;
						for (int j=0;j<data->skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0)
								num_vertices ++;
						f->write_int(num_vertices);
						for (int j=0;j<data->skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0){
								f->write_int(j);
								f->write_vector(&fr.skin[i].dpos[j]);
							}
					}
				}
			// skeletal animation
			}else if (m->type == MOVE_TYPE_SKELETAL){
				for (int j=0;j<data->bone.num;j++)
					f->write_bool((data->bone[j].parent < 0));
				f->write_bool(m->interpolated_quadratic);
				f->write_bool(m->interpolated_loop);
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->write_float(fr.duration);
					for (int j=0;j<data->bone.num;j++){
						f->write_vector(&fr.skel_ang[j]);
						if (data->bone[j].parent < 0)
							f->write_vector(&fr.skel_dpos[j]);
					}
				}
			}
		}

// effects
	f->write_comment("// Effects");
	f->write_int(data->fx.num);
	for (auto &e: data->fx){
		if (e.type == FX_TYPE_SCRIPT){
			f->write_str("Script");
			f->write_int(e.vertex);
			f->write_str(e.file);
			f->write_str("");
		}else if (e.type == FX_TYPE_LIGHT){
			f->write_str("Light");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, e.colors[nc]);
		}else if (e.type == FX_TYPE_SOUND){
			f->write_str("Sound");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			f->write_int((int)(e.speed * 100.0f));
			f->write_str(e.file);
		}else if (e.type == FX_TYPE_FORCEFIELD){
			f->write_str("ForceField");
			f->write_int(e.vertex);
			f->write_int((int)e.size);
			f->write_int((int)e.intensity);
			f->write_bool(e.inv_quad);
		}
	}

// properties
	f->write_comment("// Physics");
	f->write_float(data->meta_data.mass);
	for (int i=0;i<9;i++)
		f->write_float(data->meta_data.inertia_tensor.e[i]);
	f->write_bool(data->meta_data.active_physics);
	f->write_bool(data->meta_data.passive_physics);
	f->write_float(data->radius);

	f->write_comment("// LOD-Distances");
	f->write_float(data->meta_data.detail_dist[0]);
	f->write_float(data->meta_data.detail_dist[1]);
	f->write_float(data->meta_data.detail_dist[2]);

// object data
	f->write_comment("// Object Data");
	f->write_str(data->meta_data.name);
	f->write_str(data->meta_data.description);

	// inventory
	f->write_comment("// Inventory");
	f->write_int(data->meta_data.inventary.num);
	for (int i=0;i<data->meta_data.inventary.num;i++){
		f->write_str(data->meta_data.inventary[i]);
		f->write_int(1);
	}

	// script
	f->write_comment("// Script");
	f->write_str(data->meta_data.script_file);
	f->write_int(data->meta_data.script_var.num);
	for (int i=0;i<data->meta_data.script_var.num;i++)
		f->write_float(data->meta_data.script_var[i]);

	// new script vars
	if (data->meta_data.variables.num > 0){
		f->write_str("// Script Vars");
		f->write_int(data->meta_data.variables.num);
		for (auto &v: data->meta_data.variables){
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}


	if (data->phys_mesh->cylinder.num > 0){
		f->write_comment("// Cylinders");
		f->write_int(data->phys_mesh->cylinder.num);
		for (auto &c: data->phys_mesh->cylinder){
			f->write_int(c.index[0]);
			f->write_int(c.index[1]);
			f->write_float(c.radius);
			f->write_bool(c.round);
		}
	}

// additional data for editing
	f->write_comment("// Editor");
	f->write_bool(data->meta_data.auto_generate_tensor);
	f->write_bool(data->meta_data.auto_generate_dists);
	f->write_bool(data->meta_data.auto_generate_skin[1]);
	f->write_bool(data->meta_data.auto_generate_skin[2]);
	f->write_int(data->meta_data.detail_factor[1]);
	f->write_int(data->meta_data.detail_factor[2]);
	f->write_comment("// Normals");
	for (int i=1;i<4;i++){
		ModelSkin *s = &data->skin[i];
		f->write_int(NORMAL_MODE_PER_VERTEX);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.normal_mode);
	}
	f->write_comment("// Polygons");
	f->write_int(1);
	f->write_int(data->mesh->polygon.num);
	for (auto &t: data->mesh->polygon){
		f->write_int(t.side.num);
		f->write_int(t.material);
		for (ModelPolygonSide &ss: t.side){
			f->write_int(ss.vertex);
			for (int l=0;l<data->material[t.material]->texture_levels.num;l++){
				f->write_float(ss.skin_vertex[l].x);
				f->write_float(ss.skin_vertex[l].y);
			}
		}
	}
	f->write_bool(false);
	f->write_bool(true);
	f->write_int(0);

	f->write_comment("#");
	FileClose(f);
}

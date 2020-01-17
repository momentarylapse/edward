/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModelSurface.h"
#include "Geometry/Geometry.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Action/Action.h"
#include "../../Action/ActionManager.h"
#include "../../Edward.h"
#include "../../MultiView/MultiView.h"
#include "../../x/model_manager.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelAddVertex.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelNearifyVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelCollapseVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelAlignToGrid.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelFlattenVertices.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelTriangulateVertices.h"
#include "../../Action/Model/Mesh/Edge/ActionModelBevelEdges.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelTriangulateSelection.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelMergePolygonsSelection.h"
#include "../../Action/Model/Mesh/Polygon/ActionModelCutOutPolygons.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceVolumeSubtract.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceVolumeAnd.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfaceInvert.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAutoWeldSelection.h"
#include "../../Action/Model/Mesh/Surface/ActionModelSurfacesSubdivide.h"
#include "../../Action/Model/Mesh/Skin/ActionModelAutomap.h"
#include "../../Action/Model/Mesh/Look/ActionModelSetMaterial.h"
#include "../../Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.h"
#include "../../Action/Model/Mesh/Effects/ActionModelAddEffects.h"
#include "../../Action/Model/Mesh/Effects/ActionModelClearEffects.h"
#include "../../Action/Model/Mesh/Effects/ActionModelEditEffect.h"
#include "../../Action/Model/Mesh/ActionModelDeleteSelection.h"
#include "../../Action/Model/Mesh/ActionModelPasteGeometry.h"
#include "../../Action/Model/Mesh/ActionModelEasify.h"
#include "../../Action/Model/Animation/ActionModelAddAnimation.h"
#include "../../Action/Model/Animation/ActionModelDuplicateAnimation.h"
#include "../../Action/Model/Animation/ActionModelDeleteAnimation.h"
#include "../../Action/Model/Animation/ActionModelAnimationAddFrame.h"
#include "../../Action/Model/Animation/ActionModelAnimationDeleteFrame.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetData.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetFrameDuration.h"
#include "../../Action/Model/Animation/ActionModelAnimationSetBone.h"
#include "../../Action/Model/Skeleton/ActionModelAddBone.h"
#include "../../Action/Model/Skeleton/ActionModelAttachVerticesToBone.h"
#include "../../Action/Model/Skeleton/ActionModelDeleteBone.h"
#include "../../Action/Model/Skeleton/ActionModelDeleteBoneSelection.h"
#include "../../Action/Model/Skeleton/ActionModelReconnectBone.h"
#include "../../Action/Model/Skeleton/ActionModelSetSubModel.h"

const string DataModel::MESSAGE_SKIN_CHANGE = "SkinChange";
const string DataModel::MESSAGE_MATERIAL_CHANGE = "MaterialChange";
const string DataModel::MESSAGE_TEXTURE_CHANGE = "TextureChange";

bool DataModelAllowUpdating = true;


string ModelEffect::get_type()
{
	if (type == FX_TYPE_SCRIPT)
		return _("Script");
	if (type == FX_TYPE_LIGHT)
		return _("Licht");
	if (type == FX_TYPE_SOUND)
		return _("Sound");
	if (type == FX_TYPE_FORCEFIELD)
		return _("Kraftfeld");
	return "???";
}

void ModelEffect::clear()
{
	file = "";
	colors[0] = White;
	colors[1] = White;
	colors[2] = White;
	size = 1000.0f;
	speed = 1.0f;
	intensity = 100.0f;
	inv_quad = false;
}

ModelVertex::ModelVertex(const vector &_pos)
{
	pos = _pos;
	ref_count = 0;
	normal_mode = NORMAL_MODE_ANGULAR;
	surface = -1;
	bone_index = -1;
	normal_dirty = false;
}

DataModel::DataModel() :
	Data(FD_MODEL)
{
	radius = 100;
}

DataModel::~DataModel()
{
}


void DataModel::MetaData::reset()
{
	// level of detail
	detail_dist[0] = 2000;
	detail_dist[1] = 4000;
	detail_dist[2] = 8000;
	auto_generate_dists = true;
	detail_factor[1] = 40;
	detail_factor[2] = 20;
	auto_generate_skin[1] = false;//true;
	auto_generate_skin[2] = false;//true;

	// physics
	mass = 1;
	active_physics = true;
	passive_physics = true;
	auto_generate_tensor = true;
	inertia_tensor = matrix3::ID;

	// object data
	name = "";
	description = "";
	inventary.clear();

	// script
	script_file = "";
	script_var.clear();
	variables.clear();
}



void DataModel::reset()
{

	filename = "";
	for (int i=0;i<4;i++){
		skin[i].vertex.clear();
		for (int j=0;j<material.num;j++)
			skin[i].sub[j].triangle.clear();
		skin[i].sub.resize(1);
	}
	surface.clear();
	vertex.clear();
	ball.clear();
	cylinder.clear();
	poly.clear();
	fx.clear();

	material.clear();
	material.resize(1);
	material[0] = new ModelMaterial();
	material[0]->texture_levels.add(new ModelMaterial::TextureLevel());
	material[0]->texture_levels[0]->reload_image();
	material[0]->col.user = true;
	material[0]->col.diffuse = White;
	material[0]->col.specular = White;
	showVertices(vertex);

	// skeleton
	bone.clear();

	move.clear();


	for (int i=0;i<4;i++){
		skin[i].sub.resize(1);
		skin[i].sub[0].num_textures = 1;
	}

	radius = 42;

	meta_data.reset();

	reset_history();
	notify();
}

void DataModel::debugShow()
{
	msg_write("------------");
	msg_write(vertex.num);
	msg_write(surface.num);
	for (ModelSurface &s: surface){
		msg_write(s.polygon.num);
		s.testSanity("Model.DebugShow");
	}
}

bool DataModel::test_sanity(const string &loc)
{
	for (ModelSurface &s: surface){
		if (!s.testSanity(loc))
			return false;
	}
	return true;
}


void DataModel::on_post_action_update()
{
	showVertices(vertex);

	updateNormals();
	for (ModelSurface &s: surface){
		s.pos = v_0;
		for (int k=0;k<s.vertex.num;k++)
			s.pos += vertex[s.vertex[k]].pos;
		s.pos /= s.vertex.num;
		for (ModelPolygon &p: s.polygon){
			p.pos = v_0;
			for (int k=0;k<p.side.num;k++)
				p.pos += vertex[p.side[k].vertex].pos;
			p.pos /= p.side.num;
		}
		for (ModelEdge &e: s.edge)
			e.pos = (vertex[e.vertex[0]].pos + vertex[e.vertex[1]].pos) / 2;
	}
}

void DataModel::showVertices(Array<ModelVertex> &vert)
{
	show_vertices.set_ref(vert);
}


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


void report_error(const string &msg)
{
	msg_error(msg);
	if (ed)
		ed->set_message(msg);
}

void update_model_script_data(DataModel::MetaData &m);

bool DataModel::load(const string & _filename, bool deep)
{
	//if (allow_load)
	reset();
	bool error=false;
	int ffv;

	Array<vector> skin_vert;

	filename = _filename;
	if (ed){
		if (this == mode_model->data)
			ed->make_dirs(filename);
	}
	//msg_write(dir);
	//msg_write(filename);
	File *f;
	try{
		f = FileOpenText(filename);
	}catch(FileError &e){
		report_error(_("Datei ist nicht in der Stimmung, ge&offnet zu werden"));
		return false;
	}
	action_manager->enable(false);
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();

	if (ffv<0){
		report_error(_("Datei-Format unlesbar!"));
		error=true;

	}else if (ffv==10){ // old format

		// Materials
		f->read_comment();
		material.resize(f->read_int());
		for (int i=0;i<material.num;i++){
			material[i] = new ModelMaterial();
			material[i]->filename = f->read_str();
			material[i]->col.user = f->read_bool();
			if (material[i]->col.user){
				read_color_argb(f, material[i]->col.ambient);
				read_color_argb(f, material[i]->col.diffuse);
				read_color_argb(f, material[i]->col.specular);
				read_color_argb(f, material[i]->col.emission);
				material[i]->col.shininess = (float)f->read_int();
			}
			material[i]->alpha.mode = f->read_int();
			material[i]->alpha.user = (material[i]->alpha.mode != TransparencyModeDefault);
			material[i]->alpha.source = f->read_int();
			material[i]->alpha.destination = f->read_int();
			material[i]->alpha.factor = (float)f->read_int() * 0.01f;
			material[i]->alpha.zbuffer = f->read_bool();
			int n = f->read_int();
			material[i]->texture_levels.clear();
			for (int t=0;t<n;t++) {
				auto *tl = new ModelMaterial::TextureLevel();
				tl->filename = f->read_str();
				material[i]->texture_levels.add(tl);
			}
		}
		// create subs...
		for (int k=0;k<4;k++){
			skin[k].sub.resize(material.num);
			for (int j=0;j<material.num;j++)
				skin[k].sub[j].num_textures = 1;
		}

	// Physical Skin

		// vertices
		f->read_comment();
		skin[0].vertex.resize(f->read_int());
		for (int j=0;j<skin[0].vertex.num;j++){
			skin[0].vertex[j].bone_index = f->read_int();
			if (skin[0].vertex[j].bone_index < 0)
				skin[0].vertex[j].bone_index = 0;
			f->read_vector(&skin[0].vertex[j].pos);
		}

		// triangles
		skin[0].sub[0].triangle.resize(f->read_int());
		for (int j=0;j<skin[0].sub[0].triangle.num;j++){
			skin[0].sub[0].triangle[j].normal_dirty = true;
			for (int k=0;k<3;k++)
				skin[0].sub[0].triangle[j].vertex[k] = f->read_int();
		}

		// balls
		ball.resize(f->read_int());
		for (int j=0;j<ball.num;j++){
			ball[j].index = f->read_int();
			ball[j].radius = f->read_float();
		}

		// polys
		poly.resize(f->read_int());
		for (int j=0;j<poly.num;j++){
			poly[j].NumFaces = f->read_int();
			for (int k=0;k<poly[j].NumFaces;k++){
				poly[j].Face[k].NumVertices = f->read_int();
				for (int l=0;l<poly[j].Face[k].NumVertices;l++)
					poly[j].Face[k].Index[l] = f->read_int();
			}
		}

	// Skin[i]
		for (int i=1;i<4;i++){
			f->read_comment();
			int normal_mode_all = f->read_int();
			bool pre_normals = (normal_mode_all & NORMAL_MODE_PRE) > 0;
			normal_mode_all -= (normal_mode_all & NORMAL_MODE_PRE);

			// vertices
			skin[i].vertex.resize(f->read_int());
			for (int j=0;j<skin[i].vertex.num;j++){
				skin[i].vertex[j].bone_index = f->read_int();
				if (skin[i].vertex[j].bone_index < 0)
					skin[i].vertex[j].bone_index = 0;
				f->read_vector(&skin[i].vertex[j].pos);
				if (normal_mode_all == NORMAL_MODE_PER_VERTEX)
					skin[i].vertex[j].normal_mode = f->read_byte();
				else
					skin[i].vertex[j].normal_mode = normal_mode_all;
				skin[i].vertex[j].normal_dirty = true;
			}

			// skin vertices
			skin_vert.resize(f->read_int());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->read_float();
				skin_vert[j].y = f->read_float();
			}



			// triangles (subs)
			int num_trias = f->read_int();
			for (int t=0;t<material.num;t++)
				skin[i].sub[t].triangle.resize(f->read_int());
			for (int t=0;t<material.num;t++)
				for (int j=0;j<skin[i].sub[t].triangle.num;j++)
					for (int k=0;k<3;k++){
						skin[i].sub[t].triangle[j].vertex[k] = f->read_int();
						int svi = f->read_int();
						skin[i].sub[t].triangle[j].skin_vertex[0][k] = skin_vert[svi];
						skin[i].sub[t].triangle[j].normal_index[k] = (int)f->read_byte();
						skin[i].sub[t].triangle[j].normal_dirty = true;
					}
		}

	// Skeleton
		f->read_comment();
		bone.resize(f->read_int());
		for (int i=0;i<bone.num;i++){
			f->read_vector(&bone[i].pos);
			bone[i].parent = f->read_int();
			if (bone[i].parent > 32000)
				bone[i].parent = -1;
			if (bone[i].parent >= 0)
				bone[i].pos += bone[bone[i].parent].pos;
			bone[i].model_file = f->read_str();
			if (deep)
				bone[i].model = LoadModel(bone[i].model_file);
			bone[i].const_pos = false;
			bone[i].is_selected = bone[i].m_old = false;
		}

	// Animations
		f->read_comment();
		int num_anims = f->read_int();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->read_int();
			move.resize(anim_index + 1);
			ModelMove *m = &move[anim_index];
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
						m->frame[fr].skin[s].dpos.resize(skin[s].vertex.num);
						int num_vertices = f->read_int();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->read_int();
							f->read_vector(&m->frame[fr].skin[s].dpos[vertex_index]);
						}
					}
				}
			}else if (m->type == MOVE_TYPE_SKELETAL){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(bone.num);
				for (int j=0;j<bone.num;j++)
					VarDeltaPos[j] = f->read_bool();
				m->interpolated_quadratic = f->read_bool();
				m->interpolated_loop = f->read_bool();
				for (int fr=0;fr<m->frame.num;fr++){
					m->frame[fr].duration = 1;
					m->frame[fr].skel_dpos.resize(bone.num);
					m->frame[fr].skel_ang.resize(bone.num);
					for (int j=0;j<bone.num;j++){
						f->read_vector(&m->frame[fr].skel_ang[j]);
						if (VarDeltaPos[j])
							f->read_vector(&m->frame[fr].skel_dpos[j]);
					}
				}
			}
		}
		// Effects
		f->read_comment();
		fx.resize(f->read_int());
		if (fx.num>10000)
			fx.clear();
		for (int i=0;i<fx.num;i++){
			string fxkind = f->read_str();
			fx[i].type = -1;
			if (fxkind == "Script"){
				fx[i].type = FX_TYPE_SCRIPT;
				fx[i].vertex = f->read_int();
				fx[i].file = f->read_str();
				f->read_str();
			}
			if (fxkind == "Light"){
				fx[i].type = FX_TYPE_LIGHT;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				for (int j=0;j<3;j++)
					read_color_argb(f,fx[i].colors[j]);
			}
			if (fxkind == "Sound"){
				fx[i].type = FX_TYPE_SOUND;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				fx[i].speed = (float)f->read_int() * 0.01f;
				fx[i].file = f->read_str();
			}
			if (fxkind == "ForceField"){
				fx[i].type = FX_TYPE_FORCEFIELD;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				fx[i].intensity = (float)f->read_int();
				fx[i].inv_quad = f->read_bool();
			}
			if (fx[i].type<0)
				msg_error("unknown effekt: " + fxkind);
		}
		// LOD-Distances
		f->read_comment();
		meta_data.detail_dist[0]=f->read_float();
		meta_data.detail_dist[1]=f->read_float();
		meta_data.detail_dist[2]=f->read_float();
		meta_data.auto_generate_dists=f->read_bool();
		meta_data.detail_factor[1]=f->read_byte();
		meta_data.detail_factor[2]=f->read_byte();
		meta_data.auto_generate_skin[1]=f->read_bool();
		meta_data.auto_generate_skin[2]=f->read_bool();
		// Physics
		f->read_comment();
		meta_data.auto_generate_tensor = f->read_bool();
		for (int i=0;i<9;i++)
			meta_data.inertia_tensor.e[i] = f->read_float();
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


	}else if (ffv==11){ // new format


		// General
		vector tv;
		f->read_comment();
		// bounding box
		f->read_vector(&_min);
		f->read_vector(&_max);
		// skins
		f->read_int();
		// reserved
		f->read_int();
		f->read_int();
		f->read_int();
		//

		// Materials
		f->read_comment();
		material.resize(f->read_int());
		for (int i=0;i<material.num;i++){
			material[i] = new ModelMaterial();
			material[i]->filename = f->read_str();
			material[i]->col.user = f->read_bool();
			read_color_argb(f, material[i]->col.ambient);
			read_color_argb(f, material[i]->col.diffuse);
			read_color_argb(f, material[i]->col.specular);
			read_color_argb(f, material[i]->col.emission);
			material[i]->col.shininess = (float)f->read_int();
			material[i]->alpha.mode = f->read_int();
			material[i]->alpha.user = (material[i]->alpha.mode != TransparencyModeDefault);
			material[i]->alpha.source = f->read_int();
			material[i]->alpha.destination = f->read_int();
			material[i]->alpha.factor = (float)f->read_int() * 0.01f;
			material[i]->alpha.zbuffer = f->read_bool();
			int n = f->read_int();
			material[i]->texture_levels.clear();
			for (int t=0;t<n;t++) {
				auto tl = new ModelMaterial::TextureLevel();
				tl->filename = f->read_str();
				material[i]->texture_levels.add(tl);
			}
		}
		// create subs...
		for (int k=0;k<4;k++){
			skin[k].sub.resize(material.num);
			for (int j=0;j<material.num;j++)
				skin[k].sub[j].num_textures = material[j]->texture_levels.num;
		}

	// Physical Skin

		// vertices
		f->read_comment();
		skin[0].vertex.resize(f->read_int());
		for (int j=0;j<skin[0].vertex.num;j++)
			skin[0].vertex[j].bone_index = f->read_int();
		for (int j=0;j<skin[0].vertex.num;j++)
			f->read_vector(&skin[0].vertex[j].pos);

		// triangles
		f->read_int();

		// balls
		ball.resize(f->read_int());
		for (int j=0;j<ball.num;j++){
			ball[j].index = f->read_int();
			ball[j].radius = f->read_float();
		}

		// polys
		poly.resize(f->read_int());
		for (int j=0;j<poly.num;j++){
			poly[j].NumFaces = f->read_int();
			for (int k=0;k<poly[j].NumFaces;k++){
				poly[j].Face[k].NumVertices = f->read_int();
				for (int l=0;l<poly[j].Face[k].NumVertices;l++)
					poly[j].Face[k].Index[l] = f->read_int();
				poly[j].Face[k].Plane.n.x = f->read_float();
				poly[j].Face[k].Plane.n.y = f->read_float();
				poly[j].Face[k].Plane.n.z = f->read_float();
				poly[j].Face[k].Plane.d = f->read_float();
			}
			poly[j].NumSVertices = f->read_int();
			for (int k=0;k<poly[j].NumSVertices;k++)
				poly[j].SIndex[k] = f->read_int();
			poly[j].NumEdges = f->read_int();
			for (int k=0;k<poly[j].NumEdges;k++){
				poly[j].EdgeIndex[k*2 + 0] = f->read_int();
				poly[j].EdgeIndex[k*2 + 1] = f->read_int();
			}
			// topology
			for (int k=0;k<poly[j].NumFaces;k++)
				for (int l=0;l<poly[j].NumFaces;l++)
					poly[j].FacesJoiningEdge[k * poly[j].NumFaces + l] = f->read_int();
			for (int k=0;k<poly[j].NumEdges;k++)
				for (int l=0;l<poly[j].NumFaces;l++)
				    poly[j].EdgeOnFace[k * poly[j].NumFaces + l] = f->read_bool();
		}

	// Skin[i]
		for (int i=1;i<4;i++){

			// vertices
			f->read_comment();
			skin[i].vertex.resize(f->read_int());
			for (int j=0;j<skin[i].vertex.num;j++)
				f->read_vector(&skin[i].vertex[j].pos);
			for (int j=0;j<skin[i].vertex.num;j++)
				skin[i].vertex[j].bone_index = f->read_int();
			for (int j=0;j<skin[i].vertex.num;j++)
				skin[i].vertex[j].normal_dirty = false;//true;

			// skin vertices
			skin_vert.resize(f->read_int());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->read_float();
				skin_vert[j].y = f->read_float();
			}



			// triangles (subs)
			for (int m=0;m<material.num;m++){
				skin[i].sub[m].triangle.resize(f->read_int());
				// vertex
				for (int j=0;j<skin[i].sub[m].triangle.num;j++)
					for (int k=0;k<3;k++)
						skin[i].sub[m].triangle[j].vertex[k] = f->read_int();
				// skin vertex
				for (int tl=0;tl<material[m]->texture_levels.num;tl++)
					for (int j=0;j<skin[i].sub[m].triangle.num;j++)
						for (int k=0;k<3;k++){
							int svi = f->read_int();
							skin[i].sub[m].triangle[j].skin_vertex[tl][k] = skin_vert[svi];
						}
				// normals
				for (int j=0;j<skin[i].sub[m].triangle.num;j++){
					for (int k=0;k<3;k++){
						skin[i].sub[m].triangle[j].normal_index[k] = (int)(unsigned short)f->read_word();
						skin[i].sub[m].triangle[j].normal[k] = get_normal_by_index(skin[i].sub[m].triangle[j].normal_index[k]);
					}
					skin[i].sub[m].triangle[j].normal_dirty = false;
				}
				f->read_int();
			}
			f->read_int();
		}

	// Skeleton
		f->read_comment();
		bone.resize(f->read_int());
		for (ModelBone &b: bone){
			f->read_vector(&b.pos);
			b.parent = f->read_int();
			if ((b.parent < 0) || (b.parent >= bone.num))
				b.parent = -1;
			if (b.parent >= 0)
				b.pos += bone[b.parent].pos;
			b.model_file = f->read_str();
			if (deep)
				b.model = LoadModel(b.model_file);
			b.const_pos = false;
			b.is_selected = b.m_old = false;
		}

	// Animations
		f->read_comment();
		move.resize(f->read_int());
		int num_anims = f->read_int();
		f->read_int();
		f->read_int();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->read_int();
			move.resize(anim_index + 1);
			ModelMove *m = &move[anim_index];
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
						fr.skin[s].dpos.resize(skin[s].vertex.num);
						int num_vertices = f->read_int();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->read_int();
							f->read_vector(&fr.skin[s].dpos[vertex_index]);
						}
					}
				}
			}else if (m->type == MOVE_TYPE_SKELETAL){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(bone.num);
				for (int j=0;j<bone.num;j++)
					VarDeltaPos[j] = f->read_bool();
				m->interpolated_quadratic = f->read_bool();
				m->interpolated_loop = f->read_bool();
				for (ModelFrame &fr: m->frame){
					fr.duration = 1;
					if (rubber_timing)
						fr.duration = f->read_float();
					fr.skel_dpos.resize(bone.num);
					fr.skel_ang.resize(bone.num);
					for (int j=0;j<bone.num;j++){
						f->read_vector(&fr.skel_ang[j]);
						if (VarDeltaPos[j])
							f->read_vector(&fr.skel_dpos[j]);
					}
				}
			}else{
				msg_error("unknown animation type: " + i2s(m->type));
			}
		}
		// Effects
		f->read_comment();
		fx.resize(f->read_int());
		if (fx.num>10000)
			fx.clear();
		for (int i=0;i<fx.num;i++){
			string fxkind = f->read_str();
			fx[i].type=-1;
			if (fxkind == "Script"){
				fx[i].type = FX_TYPE_SCRIPT;
				fx[i].vertex = f->read_int();
				fx[i].file = f->read_str();
				f->read_str();
			}else if (fxkind == "Light"){
				fx[i].type = FX_TYPE_LIGHT;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				for (int j=0;j<3;j++)
					read_color_argb(f,fx[i].colors[j]);
			}else if (fxkind == "Sound"){
				fx[i].type = FX_TYPE_SOUND;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				fx[i].speed = (float)f->read_int() * 0.01f;
				fx[i].file = f->read_str();
			}else if (fxkind == "ForceField"){
				fx[i].type = FX_TYPE_FORCEFIELD;
				fx[i].vertex = f->read_int();
				fx[i].size = (float)f->read_int();
				fx[i].intensity = (float)f->read_int();
				fx[i].inv_quad = f->read_bool();
			}else{
				msg_error("unknown effekt: " + fxkind);
			}
		}

// properties
		// Physics
		f->read_comment();
		meta_data.mass = f->read_float();
		for (int i=0;i<9;i++)
			meta_data.inertia_tensor.e[i] = f->read_float();
		meta_data.active_physics = f->read_bool();
		meta_data.passive_physics = f->read_bool();
		radius = f->read_float();

		// LOD-Distances
		f->read_comment();
		meta_data.detail_dist[0] = f->read_float();
		meta_data.detail_dist[1] = f->read_float();
		meta_data.detail_dist[2] = f->read_float();

// object data
		// Object Data
		f->read_comment();
		meta_data.name = f->read_str();
		meta_data.description = f->read_str();

		// Inventary
		f->read_comment();
		meta_data.inventary.resize(f->read_int());
		for (int i=0;i<meta_data.inventary.num;i++){
			meta_data.inventary[i] = f->read_str();
			f->read_int();
		}

		// Script
		f->read_comment();
		meta_data.script_file = f->read_str();
		meta_data.script_var.resize(f->read_int());
		for (int i=0;i<meta_data.script_var.num;i++)
			meta_data.script_var[i] = f->read_float();



// optional data / additional data for editing
		while (true){
			string s = f->read_str();
			msg_write("opt:" + s);
			if (s == "// Editor"){
				meta_data.auto_generate_tensor = f->read_bool();
				meta_data.auto_generate_dists = f->read_bool();
				meta_data.auto_generate_skin[1] = f->read_bool();
				meta_data.auto_generate_skin[2] = f->read_bool();
				meta_data.detail_factor[1] = f->read_int();
				meta_data.detail_factor[2] = f->read_int();
			}else if (s == "// Normals"){
				for (int i=1;i<4;i++){
					ModelSkin *s = &skin[i];
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
				begin_action_group("LoadPolygonData");
				foreachi(ModelVertex &v, skin[1].vertex, i)
					addVertex(v.pos, v.bone_index, v.normal_mode);
				int ns = f->read_int();
				for (int i=0;i<ns;i++){
					ModelSurface s;
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
							for (int l=0;l<material[t.material]->texture_levels.num;l++){
								t.side[k].skin_vertex[l].x = f->read_float();
								t.side[k].skin_vertex[l].y = f->read_float();
							}
						}
						t.normal_dirty = true;
						s.polygon.add(t);
					}
					s.is_physical = f->read_bool();
					s.is_visible = f->read_bool();
					s.is_selected = false;
					f->read_int();
					s.model = this;
					surface.add(s);
				}
				for (ModelSurface &s: surface)
					s.buildFromPolygons();
				end_action_group();
			}else if (s == "// Cylinders"){
				int n = f->read_int();
				for (int i=0; i<n; i++){
					ModelCylinder c;
					c.index[0] = f->read_int();
					c.index[1] = f->read_int();
					c.radius = f->read_float();
					c.round = f->read_bool();
					cylinder.add(c);
				}
			}else if (s == "// Script Vars"){
				int n = f->read_int();
				for (int i=0; i<n; i++){
					ModelScriptVariable v;
					v.name = f->read_str();
					v.value = f->read_str();
					meta_data.variables.add(v);
				}
			}else if (s == "#"){
				break;
			}else{
				msg_error("unknown: " + s);
				break;
			}
		}







	}else{
		report_error(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 10, 10));
		error=true;
	}

	delete(f);








	if (deep){

		// import...
		if (surface.num == 0)
			importFromTriangleSkin(1);

		for (ModelMove &m: move)
			if (m.type == MOVE_TYPE_VERTEX){
				for (ModelFrame &f: m.frame)
					f.vertex_dpos = f.skin[1].dpos;
			}

		for (int i=0;i<material.num;i++){
			material[i]->makeConsistent();

			// test textures
			for (auto &t: material[i]->texture_levels){
				if ((!t->texture) and (t->filename.num > 0))
					report_error(format(_("Textur-Datei nicht ladbar: %s"), t->filename.c_str()));
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data){
			ed->SetTitle(filename);
			ResetView();
		}*/
	}

	// FIXME
	if ((meta_data.script_file.num > 0) and (meta_data.variables.num == 0)){
		update_model_script_data(meta_data);
		msg_write(meta_data.variables.num);
		for (int i=0; i<min(meta_data.script_var.num, meta_data.variables.num); i++){
			if (meta_data.variables[i].type == "float")
				meta_data.variables[i].value = f2s(meta_data.script_var[i], 6);
			msg_write(meta_data.variables[i].name);
			msg_write(meta_data.variables[i].value);
		}
	}


	action_manager->enable(true);
	//OptimizeView();
	reset_history();

	if (deep)
		on_post_action_update();
	return !error;
}

void DataModel::importFromTriangleSkin(int index)
{
	vertex.clear();
	surface.clear();

	ModelSkin &s = skin[index];
	begin_action_group("ImportFromTriangleSkin");
	foreachi(ModelVertex &v, s.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
	}
	for (int i=0;i<material.num;i++){
		for (ModelTriangle &t: s.sub[i].triangle){
			if ((t.vertex[0] == t.vertex[1]) || (t.vertex[1] == t.vertex[2]) || (t.vertex[2] == t.vertex[0]))
				continue;
			Array<int> v;
			for (int k=0;k<3;k++)
				v.add(t.vertex[k]);
			Array<vector> sv;
			for (int tl=0;tl<material[i]->texture_levels.num;tl++)
				for (int k=0;k<3;k++)
					sv.add(t.skin_vertex[tl][k]);
			try{
				addPolygonWithSkin(v, sv, i);
			}catch(GeometryException &e){
				msg_error("polygon..." + e.message);
				msg_write("trying to copy vertices...");

				// copy all vertices m(-_-)m
				int nv0 = vertex.num;
				for (int k=0; k<3; k++){
					addVertex(vertex[v[k]].pos);
					vertex.back().bone_index = vertex[v[k]].bone_index;
					v[k] = nv0 + k;
				}
				addPolygonWithSkin(v, sv, i);
			}
		}
	}


	ModelSkin &ps = skin[0];
	foreachi(ModelVertex &v, ps.vertex, i){
		addVertex(v.pos);
		vertex[i].bone_index = v.bone_index;
	}
	for (ModelPolyhedron &p: poly){
		msg_write("----");
		int nv0 = vertex.num;
		Array<int> vv;
		for (int i=0;i<p.NumFaces;i++){
			Array<int> v;
			for (int j=0;j<p.Face[i].NumVertices;j++){
				int vj = p.Face[i].Index[j];
				bool existing = false;
				for (int k=0;k<vv.num;k++)
					if (vv[k] == vj){
						existing = true;
						v.add(nv0 + k);
						break;
					}
				if (!existing){
					v.add(vertex.num);
					vv.add(vj);
					addVertex(ps.vertex[vj].pos);
					vertex.back().bone_index = ps.vertex[vj].bone_index;
				}
			}
			msg_write(ia2s(v));
			try{
				addPolygon(v, 0);
			}catch(GeometryException &e){
				msg_error(e.message);
			}
		}
		surface.back().is_physical = true;
		surface.back().is_visible = false;
	}
	poly.clear();

	clearSelection();
	end_action_group();
	action_manager->reset();
}

void DataModel::exportToTriangleSkin(int index)
{
	ModelSkin &sk = skin[index];
	sk.vertex = vertex;
	sk.sub.clear();
	sk.sub.resize(material.num);
	for (ModelSurface &s: surface){
		if (!s.is_visible)
			continue;
		for (ModelPolygon &t: s.polygon){
			if (t.triangulation_dirty)
				t.updateTriangulation(vertex);
			for (int i=0;i<t.side.num-2;i++){
				ModelTriangle tt;
				for (int k=0;k<3;k++){
					tt.vertex[k] = t.side[t.side[i].triangulation[k]].vertex;
					tt.normal[k] = t.side[t.side[i].triangulation[k]].normal;
					for (int l=0;l<MATERIAL_MAX_TEXTURES;l++)
						tt.skin_vertex[l][k] = t.side[t.side[i].triangulation[k]].skin_vertex[l];
				}
				sk.sub[t.material].triangle.add(tt);
			}
		}
	}
	foreachi(ModelMaterial *m, material, i)
		sk.sub[i].num_textures = m->texture_levels.num;
}


void DataModel::getBoundingBox(vector &min, vector &max)
{
	// bounding box (visual skin[1])
	min = max = v_0;
	for (int i=0;i<skin[1].vertex.num;i++){
		min._min(skin[1].vertex[i].pos);
		max._max(skin[1].vertex[i].pos);
	}
	// (physical skin)
	for (int i=0;i<skin[0].vertex.num;i++){
		min._min(skin[0].vertex[i].pos);
		max._max(skin[0].vertex[i].pos);
	}
	for (int i=0;i<ball.num;i++){
		min._min(skin[0].vertex[ball[i].index].pos - vector(1,1,1) * ball[i].radius);
		max._max(skin[0].vertex[ball[i].index].pos + vector(1,1,1) * ball[i].radius);
	}
}


bool DataModel::save(const string & _filename)
{
	if (DataModelAllowUpdating){
	/*if (AutoGenerateSkin[1])
		CreateSkin(&Skin[1],&Skin[2],(float)DetailFactor[1]*0.01f);

	if (AutoGenerateSkin[2])
		CreateSkin(&Skin[2],&Skin[3],(float)DetailFactor[2]/(float)DetailFactor[1]);*/

	if (meta_data.auto_generate_dists)
		generateDetailDists(meta_data.detail_dist);

	if (meta_data.auto_generate_tensor)
		meta_data.inertia_tensor = generateInertiaTensor(meta_data.mass);



#ifdef FORCE_UPDATE_NORMALS
	for (int d=1;d<4;d++)
		for (int j=0;j<skin[d].NumVertices;j++)
			skin[d].vertex[j].normal_dirty = true;
#endif
	updateNormals();

	// export...
	exportToTriangleSkin(1);
	for (int d=1;d<4;d++){
		if (skin[d].sub.num != material.num){
			skin[d].sub.resize(material.num);
		}
	}


//	PrecreatePhysicalData();

	getBoundingBox(_min, _max);
	radius = getRadius() * 1.1f;
	}


	// so the materials don't get mixed up
//	RemoveUnusedData();

	filename = _filename;
	if (ed)
		ed->make_dirs(filename);

	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 11);//FFVBinary, 11);
	f->float_decimals = 5;

// general
	f->write_comment("// General");
	f->write_vector(&_min);
	f->write_vector(&_max);
	f->write_int(3); // skins...
	f->write_int(0); // reserved
	f->write_int(0);
	f->write_int(0);

// materials
	f->write_comment("// Materials");
	f->write_int(material.num);
	for (ModelMaterial *m: material){
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
	f->write_int(skin[0].vertex.num);
	for (int j=0;j<skin[0].vertex.num;j++)
		f->write_int(skin[0].vertex[j].bone_index);
	for (int j=0;j<skin[0].vertex.num;j++)
		f->write_vector(&skin[0].vertex[j].pos);

	// triangles
	f->write_int(0);
	/*for (int j=0;j<Skin[0].NumTriangles;j++)
		for (int k=0;k<3;k++)
			f->write_int(Skin[0].Triangle[j].Index[k]);*/

	// balls
	f->write_int(ball.num);
	for (int j=0;j<ball.num;j++){
		f->write_int(ball[j].index);
		f->write_float(ball[j].radius);
	}

	f->write_int(poly.num);
	for (int j=0;j<poly.num;j++){
		f->write_int(poly[j].NumFaces);
		for (int k=0;k<poly[j].NumFaces;k++){
			f->write_int(poly[j].Face[k].NumVertices);
			for (int l=0;l<poly[j].Face[k].NumVertices;l++)
				f->write_int(poly[j].Face[k].Index[l]);
			f->write_float(poly[j].Face[k].Plane.n.x);
			f->write_float(poly[j].Face[k].Plane.n.y);
			f->write_float(poly[j].Face[k].Plane.n.z);
			f->write_float(poly[j].Face[k].Plane.d);
		}
		f->write_int(poly[j].NumSVertices);
		for (int k=0;k<poly[j].NumSVertices;k++)
			f->write_int(poly[j].SIndex[k]);
		f->write_int(poly[j].NumEdges);
		for (int k=0;k<poly[j].NumEdges;k++){
			f->write_int(poly[j].EdgeIndex[k*2 + 0]);
			f->write_int(poly[j].EdgeIndex[k*2 + 1]);
		}
		// topology
		for (int k=0;k<poly[j].NumFaces;k++)
			for (int l=0;l<poly[j].NumFaces;l++)
				f->write_int(poly[j].FacesJoiningEdge[k * poly[j].NumFaces + l]);
		for (int k=0;k<poly[j].NumEdges;k++)
			for (int l=0;l<poly[j].NumFaces;l++)
			    f->write_bool(poly[j].EdgeOnFace[k * poly[j].NumFaces + l]);
	}

// skin
	for (int i=1;i<4;i++){
		ModelSkin *s = &skin[i];
		f->write_comment(format("// Skin[%d]",i));

		// verices
		f->write_int(s->vertex.num);
		for (ModelVertex &v: s->vertex)
			f->write_vector(&v.pos);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.bone_index);

	    // skin vertices
		int num_skin_v = 0;
		for (int m=0;m<material.num;m++)
			num_skin_v += s->sub[m].triangle.num * material[m]->texture_levels.num * 3;
		f->write_int(num_skin_v);
		for (int m=0;m<material.num;m++)
			for (int tl=0;tl<material[m]->texture_levels.num;tl++)
		    	for (int j=0;j<s->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].x);
						f->write_float(s->sub[m].triangle[j].skin_vertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<material.num;m++){
			ModelSubSkin *sub = &s->sub[m];

			// triangles
			f->write_int(sub->triangle.num);

			// vertex index
	    	for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++)
					f->write_int(sub->triangle[j].vertex[k]);

			// skin index
			for (int tl=0;tl<material[m]->texture_levels.num;tl++)
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
	f->write_int(bone.num);
	for (ModelBone &b: bone){
		if (b.parent >= 0){
			vector dpos = b.pos - bone[b.parent].pos;
			f->write_vector(&dpos);
		}else
			f->write_vector(&b.pos);
		f->write_int(b.parent);
		f->write_str(b.model_file);
	}

// animations
	f->write_comment("// Animations");
	if ((move.num == 1) and (move[0].frame.num == 0)){
		f->write_int(0);
	}else
		f->write_int(move.num);
	int n_moves = 0;
	int n_frames_vert = 0;
	int n_frames_skel = 0;
	for (int i=0;i<move.num;i++)
		if (move[i].frame.num > 0){
			n_moves ++;
			if (move[i].type == MOVE_TYPE_VERTEX)	n_frames_vert += move[i].frame.num;
			if (move[i].type == MOVE_TYPE_SKELETAL)	n_frames_skel += move[i].frame.num;
		}
	f->write_int(n_moves);
	f->write_int(n_frames_vert);
	f->write_int(n_frames_skel);
	for (int i=0;i<move.num;i++)
		if (move[i].frame.num > 0){
			ModelMove *m = &move[i];
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
						for (int j=0;j<skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0)
								num_vertices ++;
						f->write_int(num_vertices);
						for (int j=0;j<skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0){
								f->write_int(j);
								f->write_vector(&fr.skin[i].dpos[j]);
							}
					}
				}
			// skeletal animation
			}else if (m->type == MOVE_TYPE_SKELETAL){
				for (int j=0;j<bone.num;j++)
					f->write_bool((bone[j].parent < 0));
				f->write_bool(m->interpolated_quadratic);
				f->write_bool(m->interpolated_loop);
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->write_float(fr.duration);
					for (int j=0;j<bone.num;j++){
						f->write_vector(&fr.skel_ang[j]);
						if (bone[j].parent < 0)
							f->write_vector(&fr.skel_dpos[j]);
					}
				}
			}
		}

// effects
	f->write_comment("// Effects");
	f->write_int(fx.num);
	for (int i=0;i<fx.num;i++){
		if (fx[i].type == FX_TYPE_SCRIPT){
			f->write_str("Script");
			f->write_int(fx[i].vertex);
			f->write_str(fx[i].file);
			f->write_str("");
		}else if (fx[i].type == FX_TYPE_LIGHT){
			f->write_str("Light");
			f->write_int(fx[i].vertex);
			f->write_int((int)fx[i].size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, fx[i].colors[nc]);
		}else if (fx[i].type == FX_TYPE_SOUND){
			f->write_str("Sound");
			f->write_int(fx[i].vertex);
			f->write_int((int)fx[i].size);
			f->write_int((int)(fx[i].speed * 100.0f));
			f->write_str(fx[i].file);
		}else if (fx[i].type == FX_TYPE_FORCEFIELD){
			f->write_str("ForceField");
			f->write_int(fx[i].vertex);
			f->write_int((int)fx[i].size);
			f->write_int((int)fx[i].intensity);
			f->write_bool(fx[i].inv_quad);
		}
	}

// properties
	f->write_comment("// Physics");
	f->write_float(meta_data.mass);
	for (int i=0;i<9;i++)
		f->write_float(meta_data.inertia_tensor.e[i]);
	f->write_bool(meta_data.active_physics);
	f->write_bool(meta_data.passive_physics);
	f->write_float(radius);

	f->write_comment("// LOD-Distances");
	f->write_float(meta_data.detail_dist[0]);
	f->write_float(meta_data.detail_dist[1]);
	f->write_float(meta_data.detail_dist[2]);

// object data
	f->write_comment("// Object Data");
	f->write_str(meta_data.name);
	f->write_str(meta_data.description);

	// inventary
	f->write_comment("// Inventary");
	f->write_int(meta_data.inventary.num);
	for (int i=0;i<meta_data.inventary.num;i++){
	    f->write_str(meta_data.inventary[i]);
		f->write_int(1);
	}

	// script
	f->write_comment("// Script");
	f->write_str(meta_data.script_file);
	f->write_int(meta_data.script_var.num);
	for (int i=0;i<meta_data.script_var.num;i++)
	    f->write_float(meta_data.script_var[i]);

	// new script vars
	if (meta_data.variables.num > 0){
		f->write_str("// Script Vars");
		f->write_int(meta_data.variables.num);
		for (auto &v: meta_data.variables){
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}


	if (cylinder.num > 0){
		f->write_comment("// Cylinders");
		f->write_int(cylinder.num);
		for (auto &c: cylinder){
			f->write_int(c.index[0]);
			f->write_int(c.index[1]);
			f->write_float(c.radius);
			f->write_bool(c.round);
		}
	}

// additional data for editing
	f->write_comment("// Editor");
	f->write_bool(meta_data.auto_generate_tensor);
	f->write_bool(meta_data.auto_generate_dists);
	f->write_bool(meta_data.auto_generate_skin[1]);
	f->write_bool(meta_data.auto_generate_skin[2]);
	f->write_int(meta_data.detail_factor[1]);
	f->write_int(meta_data.detail_factor[2]);
	f->write_comment("// Normals");
	for (int i=1;i<4;i++){
		ModelSkin *s = &skin[i];
		f->write_int(NORMAL_MODE_PER_VERTEX);
		for (ModelVertex &v: s->vertex)
			f->write_int(v.normal_mode);
	}
	f->write_comment("// Polygons");
	f->write_int(surface.num);
	for (ModelSurface &s: surface){
		f->write_int(s.polygon.num);
		for (ModelPolygon &t: s.polygon){
			f->write_int(t.side.num);
			f->write_int(t.material);
			for (ModelPolygonSide &ss: t.side){
				f->write_int(ss.vertex);
				for (int l=0;l<material[t.material]->texture_levels.num;l++){
					f->write_float(ss.skin_vertex[l].x);
					f->write_float(ss.skin_vertex[l].y);
				}
			}
		}
		f->write_bool(s.is_physical);
		f->write_bool(s.is_visible);
		f->write_int(0);
	}

	f->write_comment("#");
	FileClose(f);

	action_manager->mark_current_as_save();
	return true;
}

void DataModel::setNormalsDirtyByVertices(const Array<int> &index)
{
	Set<int> sindex;
	for (int i=0; i<index.num; i++)
		sindex.add(index[i]);

	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			for (int k=0;k<t.side.num;k++)
				if (!t.normal_dirty)
					if (sindex.contains(t.side[k].vertex)){
						t.normal_dirty = true;
						break;
					}
}

void DataModel::setAllNormalsDirty()
{
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			t.normal_dirty = true;
}

void DataModel::updateNormals()
{
	for (ModelSurface &s: surface)
		s.updateNormals();
}

ModelSurface *DataModel::addSurface(int surf_no)
{
	ModelSurface s;
	s.model = this;
	s.view_stage = ed->multi_view_3d->view_stage;
	s.is_selected = true;
	s.is_closed = false;
	s.is_visible = true;
	s.is_physical = true;
	if (surf_no >= 0){
		surface.insert(s, surf_no);
		return &surface[surf_no];
	}else{
		surface.add(s);
		return &surface.back();
	}
}


void DataModel::addVertex(const vector &pos, int bone_index, int normal_mode)
{	execute(new ActionModelAddVertex(pos, bone_index, normal_mode));	}

void DataModel::clearSelection()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		s.is_selected = false;
		for (ModelPolygon &t: s.polygon)
			t.is_selected = false;
		for (ModelEdge &e: s.edge)
			e.is_selected = false;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromSurfaces()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (int v: s.vertex)
			vertex[v].is_selected = s.is_selected;
		for (ModelPolygon &t: s.polygon)
			t.is_selected = s.is_selected;
		for (ModelEdge &e: s.edge)
			e.is_selected = s.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromPolygons()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (ModelEdge &e: s.edge)
			e.is_selected = false;
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected)
				for (int k=0;k<t.side.num;k++){
					vertex[t.side[k].vertex].is_selected = true;
					s.edge[t.side[k].edge].is_selected = true;
				}
	}
	for (ModelSurface &s: surface){
		s.is_selected = true;
		for (ModelPolygon &t: s.polygon)
			s.is_selected &= t.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromEdges()
{
	for (ModelVertex &v: vertex)
		v.is_selected = false;
	for (ModelSurface &s: surface){
		for (ModelEdge &e: s.edge)
			if (e.is_selected)
				for (int k=0;k<2;k++)
					vertex[e.vertex[k]].is_selected = true;
		for (ModelPolygon &p: s.polygon){
			p.is_selected = true;
			for (int k=0;k<p.side.num;k++)
				p.is_selected &= s.edge[p.side[k].edge].is_selected;
		}
		s.is_selected = true;
		for (ModelEdge &e: s.edge)
			s.is_selected &= e.is_selected;
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectionFromVertices()
{
	for (ModelSurface &s: surface){
		s.is_selected = true;
		for (ModelEdge &e: s.edge){
			e.is_selected = (vertex[e.vertex[0]].is_selected and vertex[e.vertex[1]].is_selected);
			e.view_stage = min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage);
		}
		for (ModelPolygon &t: s.polygon){
			t.is_selected = true;
			t.view_stage = vertex[t.side[0].vertex].view_stage;
			for (int k=0;k<t.side.num;k++){
				t.is_selected &= vertex[t.side[k].vertex].is_selected;
				t.view_stage = min(t.view_stage, vertex[t.side[k].vertex].view_stage);
			}
			s.is_selected &= t.is_selected;
		}
	}
	notify(MESSAGE_SELECTION);
}

void DataModel::selectOnlySurface(ModelSurface *s)
{
	for (ModelSurface &ss: surface)
		ss.is_selected = (&ss == s);
	selectionFromSurfaces();
}

ModelPolygon *DataModel::addTriangle(int a, int b, int c, int material)
{
	Array<int> v;
	v.add(a);
	v.add(b);
	v.add(c);
	Array<vector> sv;
	sv.add(vector::EY);
	sv.add(v_0);
	sv.add(vector::EX);
	return (ModelPolygon*) execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}

ModelPolygon *DataModel::addPolygon(Array<int> &v, int material)
{
	Array<vector> sv;
	for (int i=0;i<v.num;i++){
		float w = (float)i / (float)v.num * 2 * pi;
		sv.add(vector(0.5f + cos(w) * 0.5f, 0.5f + sin(w), 0));
	}
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}

ModelPolygon *DataModel::addPolygonWithSkin(Array<int> &v, Array<vector> &sv, int material)
{
	return (ModelPolygon*)execute(new ActionModelAddPolygonSingleTexture(v, material, sv));
}



int DataModel::get_surf_no(ModelSurface *s)
{
	foreachi(ModelSurface &ss, surface, i)
		if (&ss == s)
			return i;
	return -1;
}

ModelSurface *DataModel::surfaceJoin(ModelSurface *a, ModelSurface *b)
{
	a->testSanity("Join prae a");
	b->testSanity("Join prae b");

	int ai = get_surf_no(a);
	int bi = get_surf_no(b);

	// correct edge data of b
	for (ModelEdge &e: b->edge){
		if (e.polygon[0] >= 0)
			e.polygon[0] += a->polygon.num;
		if (e.polygon[1] >= 0)
			e.polygon[1] += a->polygon.num;
	}

	// correct triangle data of b
	for (ModelPolygon &t: b->polygon)
		for (int k=0;k<t.side.num;k++)
			t.side[k].edge += a->edge.num;

	// correct vertex data of b
	for (int v: b->vertex)
		vertex[v].surface = ai;

	// insert data
	a->vertex.join(b->vertex);
	a->edge.append(b->edge);
	a->polygon.append(b->polygon);

	// remove surface
	if (bi >= 0)
		surface.erase(bi);
	a = &surface[ai];
	a->testSanity("Join post a");

	return a;
}

void DataModel::createSkin(ModelSkin *src, ModelSkin *dst, float quality_factor)
{
	msg_todo("DataModel::CreateSkin");
}




float DataModel::getRadius()
{
	float radius = 0;
	for (ModelVertex &v: vertex)
		radius = max(v.pos.length(), radius);
	return radius;
}

float DetailDistTemp1,DetailDistTemp2,DetailDistTemp3;

int get_num_trias(DataModel *m, ModelSkin *s)
{
	int n = 0;
	for (int i=0;i<m->material.num;i++)
		n += s->sub[i].triangle.num;
	return n;
}

void DataModel::generateDetailDists(float *dist)
{
	float radius = getRadius();
	dist[0] = radius * 10;
	dist[1] = radius * 40;
	dist[2] = radius * 80;
	if (get_num_trias(this, &skin[3]) == 0)
		dist[1] = dist[2];
	if (get_num_trias(this, &skin[2]) == 0)
		dist[0] = dist[1];
}


#define n_theta		16

matrix3 DataModel::generateInertiaTensor(float mass)
{
//	sModeModelSkin *p = &Skin[0];

	// estimate size
	vector min = v_0, max = v_0;
	for (ModelVertex &v: vertex){
		min._min(v.pos);
		max._max(v.pos);
	}
	/*for (int i=0;i<Ball.num;i++){
		sModeModelBall *b = &Ball[i];
		vector b_min = p->Vertex[b->Index].Pos - vector(1,1,1) * b->Radius;
		vector b_max = p->Vertex[b->Index].Pos + vector(1,1,1) * b->Radius;
		VecMin(min, b_min);
		VecMax(max, b_max);
	}*/
	//msg_write(string2("	min= %f	%f	%f",min.x,min.y,min.z));
	//msg_write(string2("	max= %f	%f	%f",max.x,max.y,max.z));


	//float dv=(max.x-min.x)/n_theta*(max.y-min.y)/n_theta*(max.z-min.z)/n_theta;
	int num_ds=0;

	matrix3 t;
	for (int i=0;i<9;i++)
		t.e[i] = 0;

	for (ModelSurface &s: surface)
		s.beginInsideTests();

	for (int i=0;i<n_theta;i++){
		float x=min.x+(float(i)+0.5f)*(max.x-min.x)/n_theta;
		for (int j=0;j<n_theta;j++){
			float y=min.y+(float(j)+0.5f)*(max.y-min.y)/n_theta;
			for (int k=0;k<n_theta;k++){
				float z=min.z+(float(k)+0.5f)*(max.z-min.z)/n_theta;
				vector r=vector(x,y,z);
				//msg_write(string2("%f		%f		%f",r.x,r.y,r.z));

				bool inside=false;
				/*for (int n=0;n<Ball.num;n++){
					sModeModelBall *b=&Ball[n];
					if (VecLength(r-p->Vertex[b->Index].Pos)<b->Radius)
						inside=true;
				}*/
				for (ModelSurface &s: surface)
					if (s.insideTest(r)){
						inside = true;
						break;
					}
				if (inside){
					//msg_write("in");
					num_ds++;
					t._00 += y*y + z*z;
					t._11 += z*z + x*x;
					t._22 += x*x + y*y;
					t._01 -= x*y;
					t._12 -= y*z;
					t._20 -= z*x;
				}
			}
		}
	}


	for (ModelSurface &s: surface)
		s.endInsideTests();

	if (num_ds>0){
		float f = mass / num_ds;
		t *= f;
		t._10 = t._01;
		t._21 = t._12;
		t._02 = t._20;
	}else
		t = matrix3::ID;

	return t;
}


int DataModel::getNumSelectedVertices()
{
	int r = 0;
	/*if ((CreationMode < 0) and ((SubMode == SubModeSkeleton) || ((SubMode == SubModeAnimation) and (move->Type == MoveTypeSkeletal)))){
		for (int i=0;i<Bone.num;i++)
			if (Bone[i].IsSelected)
				r++;
		return r;
	}*/
	for (ModelVertex &v: vertex)
		if (v.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedSkinVertices()
{
	int r = 0;
	for (ModelSkinVertexDummy &v: skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedPolygons()
{
	int r = 0;
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected)
				r ++;
	return r;
}

int DataModel::getNumSelectedEdges()
{
	int r = 0;
	for (ModelSurface &s: surface)
		for (ModelEdge &e: s.edge)
			if (e.is_selected)
				r ++;
	return r;
}

int DataModel::getNumSelectedSurfaces()
{
	int r = 0;
	for (ModelSurface &s: surface)
		if (s.is_selected)
			r ++;
	return r;
}

int DataModel::getNumSelectedBones()
{
	int r = 0;
	for (ModelBone &b: bone)
		if (b.is_selected)
			r ++;
	return r;
}

int DataModel::getNumPolygons()
{
	int r = 0;
	for (ModelSurface &s: surface)
		r += s.polygon.num;
	return r;
}

void DataModel::reconnectBone(int index, int parent)
{	execute(new ActionModelReconnectBone(index, parent));	}

void DataModel::setBoneModel(int index, const string &filename)
{	execute(new ActionModelSetSubModel(index, filename));	}

void DataModel::addBone(const vector &pos, int parent)
{	execute(new ActionModelAddBone(pos, parent));	}

void DataModel::deleteBone(int index)
{	execute(new ActionModelDeleteBone(index));	}

void DataModel::deleteSelectedBones()
{	execute(new ActionModelDeleteBoneSelection(this));	}

void DataModel::boneAttachVertices(int index, const Array<int> &vertices)
{	execute(new ActionModelAttachVerticesToBone(vertices, index));	}

void DataModel::addAnimation(int index, int type)
{	execute(new ActionModelAddAnimation(index, type));	}

void DataModel::duplicateAnimation(int source, int target)
{	execute(new ActionModelDuplicateAnimation(source, target));	}

void DataModel::deleteAnimation(int index)
{	execute(new ActionModelDeleteAnimation(index));	}

void DataModel::animationAddFrame(int index, int frame, const ModelFrame &f)
{	execute(new ActionModelAnimationAddFrame(index, frame, f));	}

void DataModel::setAnimationData(int index, const string &name, float fps_const, float fps_factor)
{	execute(new ActionModelAnimationSetData(index, name, fps_const, fps_factor));	}

void DataModel::animationDeleteFrame(int index, int frame)
{	execute(new ActionModelAnimationDeleteFrame(index, frame));	}

void DataModel::animationSetFrameDuration(int index, int frame, float duration)
{	execute(new ActionModelAnimationSetFrameDuration(index, frame, duration));	}

void DataModel::animationSetBone(int move, int frame, int bone, const vector &dpos, const vector &ang)
{	execute(new ActionModelAnimationSetBone(move, frame, bone, dpos, ang));	}

void DataModel::copyGeometry(Geometry &geo)
{
	geo.clear();

	// copy vertices
	Array<int> vert;
	foreachi(ModelVertex &v, vertex, vi)
		if (v.is_selected){
			geo.vertex.add(v);
			vert.add(vi);
		}

	// copy triangles
	for (ModelSurface &s: surface)
		for (ModelPolygon &t: s.polygon)
			if (t.is_selected){
				ModelPolygon tt = t;
				for (int k=0;k<t.side.num;k++)
					foreachi(int v, vert, vi)
						if (v == t.side[k].vertex)
							tt.side[k].vertex = vi;
				geo.polygon.add(tt);
			}
}

void DataModel::deleteSelection(bool greedy)
{	execute(new ActionModelDeleteSelection(greedy));	}

void DataModel::invertSurfaces(const Set<int> &surfaces)
{	execute(new ActionModelSurfaceInvert(surfaces));	}

void DataModel::invertSelection()
{	invertSurfaces(getSelectedSurfaces());	}

void DataModel::subtractSelection()
{	execute(new ActionModelSurfaceVolumeSubtract());	}

void DataModel::andSelection()
{	execute(new ActionModelSurfaceVolumeAnd());	}

void DataModel::alignToGridSelection(float grid_d)
{	execute(new ActionModelAlignToGrid(this, grid_d));	}

void DataModel::nearifySelectedVertices()
{	execute(new ActionModelNearifyVertices(this));	}

void DataModel::collapseSelectedVertices()
{	execute(new ActionModelCollapseVertices());	}

void DataModel::setNormalModeSelection(int mode)
{	execute(new ActionModelSetNormalModeSelection(this, mode));	}

void DataModel::setMaterialSelection(int material)
{	execute(new ActionModelSetMaterial(this, material));	}

void DataModel::pasteGeometry(Geometry& geo, int default_material)
{	execute(new ActionModelPasteGeometry(geo, default_material));	}

void DataModel::easify(float factor)
{	execute(new ActionModelEasify(factor));	}

void DataModel::subdivideSelectedSurfaces()
{	execute(new ActionModelSurfacesSubdivide(getSelectedSurfaces()));	}

void DataModel::bevelSelectedEdges(float radius)
{	execute(new ActionModelBevelEdges(radius));	}

void DataModel::flattenSelectedVertices()
{	execute(new ActionModelFlattenVertices(this));	}

void DataModel::triangulateSelectedVertices()
{	execute(new ActionModelTriangulateVertices());	}

void DataModel::extrudeSelectedPolygons(float offset)
{	execute(new ActionModelExtrudePolygons(offset));	}

void DataModel::autoWeldSurfaces(const Set<int> &surfaces, float epsilon)
{	execute(new ActionModelAutoWeldSelection(epsilon));	}

void DataModel::autoWeldSelectedSurfaces(float epsilon)
{	execute(new ActionModelAutoWeldSelection(epsilon));	}

void DataModel::automap(int material, int texture_level)
{	execute(new ActionModelAutomap(material, texture_level));	}

void DataModel::selectionAddEffects(const ModelEffect& effect)
{	execute(new ActionModelAddEffects(this, effect));	}

void DataModel::editEffect(int index, const ModelEffect& effect)
{	execute(new ActionModelEditEffect(index, effect));	}

void DataModel::cutOutSelection()
{	execute(new ActionModelCutOutPolygons());	}

void DataModel::convertSelectionToTriangles()
{	execute(new ActionModelTriangulateSelection());	}

void DataModel::mergePolygonsSelection()
{	execute(new ActionModelMergePolygonsSelection());	}

void DataModel::selectionClearEffects()
{	execute(new ActionModelClearEffects(this));	}


void ModelSelectionState::clear()
{
	vertex.clear();
	surface.clear();
	polygon.clear();
}

Set<int> DataModel::getSelectedVertices()
{
	Set<int> vv;
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			vv.add(i);
	return vv;
}

Set<int> DataModel::getSelectedSurfaces()
{
	Set<int> ss;
	foreachi(ModelSurface &surf, surface, i)
		if (surf.is_selected)
			ss.add(i);
	return ss;
}

void DataModel::getSelectionState(ModelSelectionState& s)
{
	s.clear();
	foreachi(ModelVertex &v, vertex, i)
		if (v.is_selected)
			s.vertex.add(i);
	foreachi(ModelSurface &surf, surface, i){
		if (surf.is_selected)
			s.surface.add(i);
		Set<int> sel;
		foreachi(ModelPolygon &t, surf.polygon, j)
			if (t.is_selected)
				sel.add(j);
		s.polygon.add(sel);
		Array<ModelSelectionState::EdgeSelection> esel;
		foreachi(ModelEdge &e, surf.edge, j)
			if (e.is_selected)
				esel.add(e.vertex);
		s.edge.add(esel);
	}
}

void DataModel::setSelectionState(ModelSelectionState& s)
{
	clearSelection();
	for (int v: s.vertex)
		vertex[v].is_selected = true;
	for (int si: s.surface)
		surface[si].is_selected = true;
	for (int i=0;i<s.polygon.num;i++)
		for (int j: s.polygon[i])
			surface[i].polygon[j].is_selected = true;
	for (int i=0;i<s.edge.num;i++)
		for (ModelSelectionState::EdgeSelection &es: s.edge[i]){
			int ne = surface[i].findEdge(es.v[0], es.v[1]);
			if (ne >= 0)
				surface[i].edge[ne].is_selected = true;
		}
	notify(MESSAGE_SELECTION);
}






/*int DataModel::GetNumMarkedBalls()
{
	int r=0;
	for (int i=0;i<Ball.num;i++)
		if (Ball[i].IsSelected)
			r++;
	return r;
}

int DataModel::GetNumMarkedKonvPolys()
{
	int r=0;
	for (int i=0;i<Poly.num;i++)
		if (Poly[i].IsSelected)
			r++;
	return r;
 }*/

ModelSelectionState::EdgeSelection::EdgeSelection(int _v[2])
{
	v[0] = _v[0];
	v[1] = _v[1];
}

float ModelMove::duration()
{
	float t = 0;
	for (ModelFrame &f: frame)
		t += f.duration;
	return t;
}

bool ModelMove::needsRubberTiming()
{
	for (ModelFrame &f: frame)
		if (fabs(f.duration - 1.0f) > 0.01f)
			return true;
	return false;
}

void ModelMove::getTimeInterpolation(float time, int &frame0, int &frame1, float &t)
{
	float t0 = 0;
	foreachi(ModelFrame &f, frame, i){
		if (time < t0 + f.duration){
			frame0 = i;
			frame1 = (i + 1) % frame.num;
			t = (time - t0) / f.duration;
			return;
		}
		t0 += f.duration;
	}
	frame0 = 0;
	frame1 = 0;
	t = 0;
}

ModelFrame ModelMove::interpolate(float time)
{
	ModelFrame f;
	float t;
	int frame0, frame1;
	getTimeInterpolation(time, frame0, frame1, t);
	ModelFrame &f0 = frame[frame0];
	ModelFrame &f1 = frame[frame1];

	if (type == MOVE_TYPE_VERTEX){
		int n = f0.vertex_dpos.num;
		f.vertex_dpos.resize(n);
		for (int i=0; i<n; i++)
			f.vertex_dpos[i] = (1 - t) * f0.vertex_dpos[i] + t * f1.vertex_dpos[i];
	}else if (type == MOVE_TYPE_SKELETAL){
		int n = f0.skel_ang.num;
		f.skel_ang.resize(n);
		f.skel_dpos.resize(n);
		for (int i=0; i<n; i++){
			f.skel_dpos[i] = (1 - t) * f0.skel_dpos[i] + t * f1.skel_dpos[i];
			quaternion q0, q1, q;
			q0 = quaternion::rotation_v( f0.skel_ang[i]);
			q1 = quaternion::rotation_v( f1.skel_ang[i]);
			q = quaternion::interpolate( q0, q1, t);
			f.skel_ang[i] = q.get_angles();
		}
	}
	return f;
}

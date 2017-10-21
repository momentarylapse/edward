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

const string DataModel::MESSAGE_SELECTION = "Selection";
const string DataModel::MESSAGE_SKIN_CHANGE = "SkinChange";


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


DataModel::DataModel() :
	Data(FD_MODEL)
{
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
	Matrix3Identity(inertia_tensor);

	// object data
	name = "";
	description = "";
	inventary.clear();

	// script
	script_file = "";
	script_var.clear();
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
	poly.clear();
	fx.clear();
	material.resize(1);
	material[0].reset();
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

	resetHistory();
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

bool DataModel::testSanity(const string &loc)
{
	for (ModelSurface &s: surface){
		if (!s.testSanity(loc))
			return false;
	}
	return true;
}


void DataModel::onPostActionUpdate()
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
	if ((n.x == 0) && (n.y == 0)){
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




bool DataModel::load(const string & _filename, bool deep)
{
	msg_db_f("Model::LoadFromFile",1);
	//if (allow_load)
	reset();
	bool error=false;
	int ffv;

	Array<vector> skin_vert;

	filename = _filename;
	if (this == mode_model->data)
		ed->makeDirs(filename);
	//msg_write(dir);
	//msg_write(filename);
	File *f = FileOpen(filename);
	if (!f){
		ed->setMessage(_("Datei ist nicht in der Stimmung, ge&offnet zu werden"));
		return false;
	}
	action_manager->enable(false);
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();

	if (ffv<0){
		ed->errorBox(_("Datei-Format unlesbar!"));
		error=true;

	}else if (ffv==10){ // old format

		// Materials
		f->ReadComment();
		material.resize(f->ReadInt());
		for (int i=0;i<material.num;i++){
			material[i].material_file = f->ReadStr();
			material[i].user_color = f->ReadBool();
			if (material[i].user_color){
				read_color_argb(f, material[i].ambient);
				read_color_argb(f, material[i].diffuse);
				read_color_argb(f, material[i].specular);
				read_color_argb(f, material[i].emission);
				material[i].shininess = (float)f->ReadInt();
			}
			material[i].transparency_mode = f->ReadInt();
			material[i].user_transparency = (material[i].transparency_mode != TransparencyModeDefault);
			material[i].alpha_source = f->ReadInt();
			material[i].alpha_destination = f->ReadInt();
			material[i].alpha_factor = (float)f->ReadInt() * 0.01f;
			material[i].alpha_zbuffer = f->ReadBool();
			material[i].num_textures = f->ReadInt();
			for (int t=0;t<material[i].num_textures;t++)
				material[i].texture_file[t] = f->ReadStr();
		}
		// create subs...
		for (int k=0;k<4;k++){
			skin[k].sub.resize(material.num);
			for (int j=0;j<material.num;j++)
				skin[k].sub[j].num_textures = 1;
		}

	// Physical Skin

		// vertices
		f->ReadComment();
		skin[0].vertex.resize(f->ReadInt());
		for (int j=0;j<skin[0].vertex.num;j++){
			skin[0].vertex[j].bone_index = f->ReadInt();
			if (skin[0].vertex[j].bone_index < 0)
				skin[0].vertex[j].bone_index = 0;
			f->ReadVector(&skin[0].vertex[j].pos);
		}

		// triangles
		skin[0].sub[0].triangle.resize(f->ReadInt());
		for (int j=0;j<skin[0].sub[0].triangle.num;j++){
			skin[0].sub[0].triangle[j].normal_dirty = true;
			for (int k=0;k<3;k++)
				skin[0].sub[0].triangle[j].vertex[k] = f->ReadInt();
		}

		// balls
		ball.resize(f->ReadInt());
		for (int j=0;j<ball.num;j++){
			ball[j].index = f->ReadInt();
			ball[j].radius = f->ReadFloat();
		}

		// polys
		poly.resize(f->ReadInt());
		for (int j=0;j<poly.num;j++){
			poly[j].NumFaces = f->ReadInt();
			for (int k=0;k<poly[j].NumFaces;k++){
				poly[j].Face[k].NumVertices = f->ReadInt();
				for (int l=0;l<poly[j].Face[k].NumVertices;l++)
					poly[j].Face[k].Index[l] = f->ReadInt();
			}
		}

	// Skin[i]
		for (int i=1;i<4;i++){
			f->ReadComment();
			int normal_mode_all = f->ReadInt();
			bool pre_normals = (normal_mode_all & NORMAL_MODE_PRE) > 0;
			normal_mode_all -= (normal_mode_all & NORMAL_MODE_PRE);

			// vertices
			skin[i].vertex.resize(f->ReadInt());
			for (int j=0;j<skin[i].vertex.num;j++){
				skin[i].vertex[j].bone_index = f->ReadInt();
				if (skin[i].vertex[j].bone_index < 0)
					skin[i].vertex[j].bone_index = 0;
				f->ReadVector(&skin[i].vertex[j].pos);
				if (normal_mode_all == NORMAL_MODE_PER_VERTEX)
					skin[i].vertex[j].normal_mode = f->ReadByte();
				else
					skin[i].vertex[j].normal_mode = normal_mode_all;
				skin[i].vertex[j].normal_dirty = true;
			}

			// skin vertices
			skin_vert.resize(f->ReadInt());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->ReadFloat();
				skin_vert[j].y = f->ReadFloat();
			}



			// triangles (subs)
			int num_trias = f->ReadInt();
			for (int t=0;t<material.num;t++)
				skin[i].sub[t].triangle.resize(f->ReadInt());
			for (int t=0;t<material.num;t++)
				for (int j=0;j<skin[i].sub[t].triangle.num;j++)
					for (int k=0;k<3;k++){
						skin[i].sub[t].triangle[j].vertex[k] = f->ReadInt();
						int svi = f->ReadInt();
						skin[i].sub[t].triangle[j].skin_vertex[0][k] = skin_vert[svi];
						skin[i].sub[t].triangle[j].normal_index[k] = (int)f->ReadByte();
						skin[i].sub[t].triangle[j].normal_dirty = true;
					}
		}

	// Skeleton
		f->ReadComment();
		bone.resize(f->ReadInt());
		for (int i=0;i<bone.num;i++){
			f->ReadVector(&bone[i].pos);
			bone[i].parent = f->ReadInt();
			if (bone[i].parent > 32000)
				bone[i].parent = -1;
			if (bone[i].parent >= 0)
				bone[i].pos += bone[bone[i].parent].pos;
			bone[i].model_file = f->ReadStr();
			if (deep)
				bone[i].model = LoadModel(bone[i].model_file);
			bone[i].const_pos = false;
			bone[i].is_selected = bone[i].m_old = false;
		}

	// Animations
		f->ReadComment();
		int num_anims = f->ReadInt();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->ReadInt();
			move.resize(anim_index + 1);
			ModelMove *m = &move[anim_index];
			m->name = f->ReadStr();
			m->type = f->ReadInt();
			m->frame.resize(f->ReadInt());
			m->frames_per_sec_const = f->ReadFloat();
			m->frames_per_sec_factor = f->ReadFloat();

			// vertex animation
			if (m->type == MOVE_TYPE_VERTEX){
				for (int fr=0;fr<m->frame.num;fr++){
					m->frame[fr].duration = 1;
					for (int s=0;s<4;s++){
						m->frame[fr].skin[s].dpos.resize(skin[s].vertex.num);
						int num_vertices = f->ReadInt();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->ReadInt();
							f->ReadVector(&m->frame[fr].skin[s].dpos[vertex_index]);
						}
					}
				}
			}else if (m->type == MOVE_TYPE_SKELETAL){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(bone.num);
				for (int j=0;j<bone.num;j++)
					VarDeltaPos[j] = f->ReadBool();
				m->interpolated_quadratic = f->ReadBool();
				m->interpolated_loop = f->ReadBool();
				for (int fr=0;fr<m->frame.num;fr++){
					m->frame[fr].duration = 1;
					m->frame[fr].skel_dpos.resize(bone.num);
					m->frame[fr].skel_ang.resize(bone.num);
					for (int j=0;j<bone.num;j++){
						f->ReadVector(&m->frame[fr].skel_ang[j]);
						if (VarDeltaPos[j])
							f->ReadVector(&m->frame[fr].skel_dpos[j]);
					}
				}
			}
		}
		// Effects
		f->ReadComment();
		fx.resize(f->ReadInt());
		if (fx.num>10000)
			fx.clear();
		for (int i=0;i<fx.num;i++){
			string fxkind = f->ReadStr();
			fx[i].type = -1;
			if (fxkind == "Script"){
				fx[i].type = FX_TYPE_SCRIPT;
				fx[i].vertex = f->ReadInt();
				fx[i].file = f->ReadStr();
				f->ReadStr();
			}
			if (fxkind == "Light"){
				fx[i].type = FX_TYPE_LIGHT;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				for (int j=0;j<3;j++)
					read_color_argb(f,fx[i].colors[j]);
			}
			if (fxkind == "Sound"){
				fx[i].type = FX_TYPE_SOUND;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				fx[i].speed = (float)f->ReadInt() * 0.01f;
				fx[i].file = f->ReadStr();
			}
			if (fxkind == "ForceField"){
				fx[i].type = FX_TYPE_FORCEFIELD;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				fx[i].intensity = (float)f->ReadInt();
				fx[i].inv_quad = f->ReadBool();
			}
			if (fx[i].type<0)
				msg_error("unknown effekt: " + fxkind);
		}
		// LOD-Distances
		f->ReadComment();
		meta_data.detail_dist[0]=f->ReadFloat();
		meta_data.detail_dist[1]=f->ReadFloat();
		meta_data.detail_dist[2]=f->ReadFloat();
		meta_data.auto_generate_dists=f->ReadBool();
		meta_data.detail_factor[1]=f->ReadByte();
		meta_data.detail_factor[2]=f->ReadByte();
		meta_data.auto_generate_skin[1]=f->ReadBool();
		meta_data.auto_generate_skin[2]=f->ReadBool();
		// Physics
		f->ReadComment();
		meta_data.auto_generate_tensor = f->ReadBool();
		for (int i=0;i<9;i++)
			meta_data.inertia_tensor.e[i] = f->ReadFloat();
		// BG Textures
		/*if (strcmp(f->ReadStr(),"#")!=0){
			for (int i=0;i<4;i++){
				char sss[512];
				strcpy(sss,f->ReadStr());
				if (strlen(sss)>0){
					strcpy(BgTextureFile[i],sss);
					if (allow_load)
						BgTexture[i]=NixLoadTexture(sss);
					BgTextureA[i].x=f->ReadFloat();
					BgTextureA[i].y=f->ReadFloat();
					BgTextureA[i].z=f->ReadFloat();
					BgTextureB[i].x=f->ReadFloat();
					BgTextureB[i].y=f->ReadFloat();
					BgTextureB[i].z=f->ReadFloat();
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
		AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);*/


	}else if (ffv==11){ // new format


		// General
		vector tv;
		f->ReadComment();
		// bounding box
		f->ReadVector(&tv);
		f->ReadVector(&tv);
		// skins
		f->ReadInt();
		// reserved
		f->ReadInt();
		f->ReadInt();
		f->ReadInt();
		//

		// Materials
		f->ReadComment();
		material.resize(f->ReadInt());
		for (int i=0;i<material.num;i++){
			material[i].material_file = f->ReadStr();
			material[i].user_color = f->ReadBool();
			read_color_argb(f, material[i].ambient);
			read_color_argb(f, material[i].diffuse);
			read_color_argb(f, material[i].specular);
			read_color_argb(f, material[i].emission);
			material[i].shininess = (float)f->ReadInt();
			material[i].transparency_mode = f->ReadInt();
			material[i].user_transparency = (material[i].transparency_mode != TransparencyModeDefault);
			material[i].alpha_source = f->ReadInt();
			material[i].alpha_destination = f->ReadInt();
			material[i].alpha_factor = (float)f->ReadInt() * 0.01f;
			material[i].alpha_zbuffer = f->ReadBool();
			material[i].num_textures = f->ReadInt();
			for (int t=0;t<material[i].num_textures;t++)
				material[i].texture_file[t] = f->ReadStr();
		}
		// create subs...
		for (int k=0;k<4;k++){
			skin[k].sub.resize(material.num);
			for (int j=0;j<material.num;j++)
				skin[k].sub[j].num_textures = material[j].num_textures;
		}

	// Physical Skin

		// vertices
		f->ReadComment();
		skin[0].vertex.resize(f->ReadInt());
		for (int j=0;j<skin[0].vertex.num;j++)
			skin[0].vertex[j].bone_index = f->ReadInt();
		for (int j=0;j<skin[0].vertex.num;j++)
			f->ReadVector(&skin[0].vertex[j].pos);

		// triangles
		f->ReadInt();

		// balls
		ball.resize(f->ReadInt());
		for (int j=0;j<ball.num;j++){
			ball[j].index = f->ReadInt();
			ball[j].radius = f->ReadFloat();
		}

		// polys
		poly.resize(f->ReadInt());
		for (int j=0;j<poly.num;j++){
			poly[j].NumFaces = f->ReadInt();
			for (int k=0;k<poly[j].NumFaces;k++){
				poly[j].Face[k].NumVertices = f->ReadInt();
				for (int l=0;l<poly[j].Face[k].NumVertices;l++)
					poly[j].Face[k].Index[l] = f->ReadInt();
				f->ReadFloat();
				f->ReadFloat();
				f->ReadFloat();
				f->ReadFloat();
			}
			poly[j].NumSVertices = f->ReadInt();
			for (int k=0;k<poly[j].NumSVertices;k++)
				f->ReadInt();
			poly[j].NumEdges = f->ReadInt();
			for (int k=0;k<poly[j].NumEdges*2;k++)
				f->ReadInt();
			// topology
			for (int k=0;k<poly[j].NumFaces*poly[j].NumFaces;k++)
				f->ReadInt();
			for (int k=0;k<poly[j].NumEdges*poly[j].NumFaces;k++)
				f->ReadBool();
		}

	// Skin[i]
		for (int i=1;i<4;i++){

			// vertices
			f->ReadComment();
			skin[i].vertex.resize(f->ReadInt());
			for (int j=0;j<skin[i].vertex.num;j++)
				f->ReadVector(&skin[i].vertex[j].pos);
			for (int j=0;j<skin[i].vertex.num;j++)
				skin[i].vertex[j].bone_index = f->ReadInt();
			for (int j=0;j<skin[i].vertex.num;j++)
				skin[i].vertex[j].normal_dirty = false;//true;

			// skin vertices
			skin_vert.resize(f->ReadInt());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->ReadFloat();
				skin_vert[j].y = f->ReadFloat();
			}



			// triangles (subs)
			for (int m=0;m<material.num;m++){
				skin[i].sub[m].triangle.resize(f->ReadInt());
				// vertex
				for (int j=0;j<skin[i].sub[m].triangle.num;j++)
					for (int k=0;k<3;k++)
						skin[i].sub[m].triangle[j].vertex[k] = f->ReadInt();
				// skin vertex
				for (int tl=0;tl<material[m].num_textures;tl++)
					for (int j=0;j<skin[i].sub[m].triangle.num;j++)
						for (int k=0;k<3;k++){
							int svi = f->ReadInt();
							skin[i].sub[m].triangle[j].skin_vertex[tl][k] = skin_vert[svi];
						}
				// normals
				for (int j=0;j<skin[i].sub[m].triangle.num;j++){
					for (int k=0;k<3;k++){
						skin[i].sub[m].triangle[j].normal_index[k] = (int)(unsigned short)f->ReadWord();
						skin[i].sub[m].triangle[j].normal[k] = get_normal_by_index(skin[i].sub[m].triangle[j].normal_index[k]);
					}
					skin[i].sub[m].triangle[j].normal_dirty = false;
				}
				f->ReadInt();
			}
			f->ReadInt();
		}

	// Skeleton
		f->ReadComment();
		bone.resize(f->ReadInt());
		for (ModelBone &b: bone){
			f->ReadVector(&b.pos);
			b.parent = f->ReadInt();
			if ((b.parent < 0) || (b.parent >= bone.num))
				b.parent = -1;
			if (b.parent >= 0)
				b.pos += bone[b.parent].pos;
			b.model_file = f->ReadStr();
			if (deep)
				b.model = LoadModel(b.model_file);
			b.const_pos = false;
			b.is_selected = b.m_old = false;
		}

	// Animations
		f->ReadComment();
		move.resize(f->ReadInt());
		int num_anims = f->ReadInt();
		f->ReadInt();
		f->ReadInt();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->ReadInt();
			move.resize(anim_index + 1);
			ModelMove *m = &move[anim_index];
			m->name = f->ReadStr();
			m->type = f->ReadInt();
			bool rubber_timing = (m->type & 128);
			m->type = m->type & 0x7f;
			m->frame.resize(f->ReadInt());
			m->frames_per_sec_const = f->ReadFloat();
			m->frames_per_sec_factor = f->ReadFloat();

			// vertex animation
			if (m->type == MOVE_TYPE_VERTEX){
				for (ModelFrame &fr: m->frame){
					fr.duration = 1;
					if (rubber_timing)
						fr.duration = f->ReadFloat();
					for (int s=0;s<4;s++){
						fr.skin[s].dpos.resize(skin[s].vertex.num);
						int num_vertices = f->ReadInt();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->ReadInt();
							f->ReadVector(&fr.skin[s].dpos[vertex_index]);
						}
					}
				}
			}else if (m->type == MOVE_TYPE_SKELETAL){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(bone.num);
				for (int j=0;j<bone.num;j++)
					VarDeltaPos[j] = f->ReadBool();
				m->interpolated_quadratic = f->ReadBool();
				m->interpolated_loop = f->ReadBool();
				for (ModelFrame &fr: m->frame){
					fr.duration = 1;
					if (rubber_timing)
						fr.duration = f->ReadFloat();
					fr.skel_dpos.resize(bone.num);
					fr.skel_ang.resize(bone.num);
					for (int j=0;j<bone.num;j++){
						f->ReadVector(&fr.skel_ang[j]);
						if (VarDeltaPos[j])
							f->ReadVector(&fr.skel_dpos[j]);
					}
				}
			}else{
				msg_error("unknown animation type: " + i2s(m->type));
			}
		}
		// Effects
		f->ReadComment();
		fx.resize(f->ReadInt());
		if (fx.num>10000)
			fx.clear();
		for (int i=0;i<fx.num;i++){
			string fxkind = f->ReadStr();
			fx[i].type=-1;
			if (fxkind == "Script"){
				fx[i].type = FX_TYPE_SCRIPT;
				fx[i].vertex = f->ReadInt();
				fx[i].file = f->ReadStr();
				f->ReadStr();
			}else if (fxkind == "Light"){
				fx[i].type = FX_TYPE_LIGHT;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				for (int j=0;j<3;j++)
					read_color_argb(f,fx[i].colors[j]);
			}else if (fxkind == "Sound"){
				fx[i].type = FX_TYPE_SOUND;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				fx[i].speed = (float)f->ReadInt() * 0.01f;
				fx[i].file = f->ReadStr();
			}else if (fxkind == "ForceField"){
				fx[i].type = FX_TYPE_FORCEFIELD;
				fx[i].vertex = f->ReadInt();
				fx[i].size = (float)f->ReadInt();
				fx[i].intensity = (float)f->ReadInt();
				fx[i].inv_quad = f->ReadBool();
			}else{
				msg_error("unknown effekt: " + fxkind);
			}
		}

// properties
		// Physics
		f->ReadComment();
		meta_data.mass = f->ReadFloat();
		for (int i=0;i<9;i++)
			meta_data.inertia_tensor.e[i] = f->ReadFloat();
		meta_data.active_physics = f->ReadBool();
		meta_data.passive_physics = f->ReadBool();
		radius = f->ReadFloat();

		// LOD-Distances
		f->ReadComment();
		meta_data.detail_dist[0] = f->ReadFloat();
		meta_data.detail_dist[1] = f->ReadFloat();
		meta_data.detail_dist[2] = f->ReadFloat();

// object data
		// Object Data
		f->ReadComment();
		meta_data.name = f->ReadStr();
		meta_data.description = f->ReadStr();

		// Inventary
		f->ReadComment();
		meta_data.inventary.resize(f->ReadInt());
		for (int i=0;i<meta_data.inventary.num;i++){
			meta_data.inventary[i] = f->ReadStr();
			f->ReadInt();
		}

		// Script
		f->ReadComment();
		meta_data.script_file = f->ReadStr();
		meta_data.script_var.resize(f->ReadInt());
		for (int i=0;i<meta_data.script_var.num;i++)
			meta_data.script_var[i] = f->ReadFloat();


// additional data for editing
		// Editor
		f->ReadComment();
		meta_data.auto_generate_tensor = f->ReadBool();
		meta_data.auto_generate_dists = f->ReadBool();
		meta_data.auto_generate_skin[1] = f->ReadBool();
		meta_data.auto_generate_skin[2] = f->ReadBool();
		meta_data.detail_factor[1] = f->ReadInt();
		meta_data.detail_factor[2] = f->ReadInt();
		// Normals
		f->ReadComment();
		for (int i=1;i<4;i++){
			ModelSkin *s = &skin[i];
			int normal_mode_all = f->ReadInt();
			if (normal_mode_all == NORMAL_MODE_PER_VERTEX){
				for (ModelVertex &v: s->vertex)
					v.normal_mode = f->ReadInt();
			}else{
				for (ModelVertex &v: s->vertex)
					v.normal_mode = normal_mode_all;
			}
		}

		// Polygons
		if (f->ReadStr() == "// Polygons"){
			beginActionGroup("LoadPolygonData");
			foreachi(ModelVertex &v, skin[1].vertex, i)
				addVertex(v.pos, v.bone_index, v.normal_mode);
			int ns = f->ReadInt();
			for (int i=0;i<ns;i++){
				ModelSurface s;
				int nv = f->ReadInt();
				for (int j=0;j<nv;j++){
					ModelPolygon t;
					t.is_selected = false;
					t.triangulation_dirty = true;
					int n = f->ReadInt();
					t.material = f->ReadInt();
					t.side.resize(n);
					for (int k=0;k<n;k++){
						t.side[k].vertex = f->ReadInt();
						for (int l=0;l<material[t.material].num_textures;l++){
							t.side[k].skin_vertex[l].x = f->ReadFloat();
							t.side[k].skin_vertex[l].y = f->ReadFloat();
						}
					}
					t.normal_dirty = true;
					s.polygon.add(t);
				}
				s.is_physical = f->ReadBool();
				s.is_visible = f->ReadBool();
				s.is_selected = false;
				f->ReadInt();
				s.model = this;
				surface.add(s);
			}
			for (ModelSurface &s: surface)
				s.buildFromPolygons();
			endActionGroup();
		}







	}else{
		ed->errorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 10, 10));
		error=true;
	}

	f->Close();
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
			material[i].makeConsistent();

			// test textures
			for (int t=0;t<material[i].num_textures;t++){
				if ((material[i].texture[t] < 0) && (material[i].texture_file[t].num > 0))
					ed->setMessage(format(_("Textur-Datei nicht ladbar: %s"), material[i].texture_file[t].c_str()));
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data){
			ed->SetTitle(filename);
			ResetView();
		}*/
	}


	action_manager->enable(true);
	//OptimizeView();
	resetHistory();

	if (deep)
		onPostActionUpdate();
	return !error;
}

void DataModel::importFromTriangleSkin(int index)
{
	vertex.clear();
	surface.clear();

	ModelSkin &s = skin[index];
	beginActionGroup("ImportFromTriangleSkin");
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
			for (int tl=0;tl<material[i].num_textures;tl++)
				for (int k=0;k<3;k++)
					sv.add(t.skin_vertex[tl][k]);
			addPolygonWithSkin(v, sv, i);
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
	endActionGroup();
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
	foreachi(ModelMaterial &m, material, i)
		sk.sub[i].num_textures = m.num_textures;
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
	msg_db_f("DataModel.Save",1);

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


	// so the materials don't get mixed up
//	RemoveUnusedData();

	filename = _filename;
	ed->makeDirs(filename);

	File *f = FileCreate(filename);
	f->WriteFileFormatVersion(false, 11);//FFVBinary, 11);
	f->FloatDecimals = 5;

// general
	f->WriteComment("// General");
	f->WriteVector(&_min);
	f->WriteVector(&_max);
	f->WriteInt(3); // skins...
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);

// materials
	f->WriteComment("// Materials");
	f->WriteInt(material.num);
	for (ModelMaterial &m: material){
		f->WriteStr(m.material_file);
		f->WriteBool(m.user_color);
		write_color_argb(f, m.ambient);
		write_color_argb(f, m.diffuse);
		write_color_argb(f, m.specular);
		write_color_argb(f, m.emission);
		f->WriteInt(m.shininess);
		f->WriteInt(m.user_transparency ? m.transparency_mode : TransparencyModeDefault);
		f->WriteInt(m.alpha_source);
		f->WriteInt(m.alpha_destination);
		f->WriteInt(m.alpha_factor * 100.0f);
		f->WriteBool(m.alpha_zbuffer);
		f->WriteInt(m.num_textures);
		for (int t=0;t<m.num_textures;t++)
			f->WriteStr(m.texture_file[t]);
	}

// physical skin
	f->WriteComment("// Physical Skin");

	// vertices
	f->WriteInt(skin[0].vertex.num);
	for (int j=0;j<skin[0].vertex.num;j++)
		f->WriteInt(skin[0].vertex[j].bone_index);
	for (int j=0;j<skin[0].vertex.num;j++)
		f->WriteVector(&skin[0].vertex[j].pos);

	// triangles
	f->WriteInt(0);
	/*for (int j=0;j<Skin[0].NumTriangles;j++)
		for (int k=0;k<3;k++)
			f->WriteInt(Skin[0].Triangle[j].Index[k]);*/

	// balls
	f->WriteInt(ball.num);
	for (int j=0;j<ball.num;j++){
		f->WriteInt(ball[j].index);
		f->WriteFloat(ball[j].radius);
	}

	f->WriteInt(poly.num);
	for (int j=0;j<poly.num;j++){
		f->WriteInt(poly[j].NumFaces);
		for (int k=0;k<poly[j].NumFaces;k++){
			f->WriteInt(poly[j].Face[k].NumVertices);
			for (int l=0;l<poly[j].Face[k].NumVertices;l++)
				f->WriteInt(poly[j].Face[k].Index[l]);
			f->WriteFloat(poly[j].Face[k].Plane.n.x);
			f->WriteFloat(poly[j].Face[k].Plane.n.y);
			f->WriteFloat(poly[j].Face[k].Plane.n.z);
			f->WriteFloat(poly[j].Face[k].Plane.d);
		}
		f->WriteInt(poly[j].NumSVertices);
		for (int k=0;k<poly[j].NumSVertices;k++)
			f->WriteInt(poly[j].SIndex[k]);
		f->WriteInt(poly[j].NumEdges);
		for (int k=0;k<poly[j].NumEdges;k++){
			f->WriteInt(poly[j].EdgeIndex[k*2 + 0]);
			f->WriteInt(poly[j].EdgeIndex[k*2 + 1]);
		}
		// topology
		for (int k=0;k<poly[j].NumFaces;k++)
			for (int l=0;l<poly[j].NumFaces;l++)
				f->WriteInt(poly[j].FacesJoiningEdge[k * poly[j].NumFaces + l]);
		for (int k=0;k<poly[j].NumEdges;k++)
			for (int l=0;l<poly[j].NumFaces;l++)
			    f->WriteBool(poly[j].EdgeOnFace[k * poly[j].NumFaces + l]);
	}

// skin
	for (int i=1;i<4;i++){
		ModelSkin *s = &skin[i];
		f->WriteComment(format("// Skin[%d]",i));

		// verices
		f->WriteInt(s->vertex.num);
		for (ModelVertex &v: s->vertex)
			f->WriteVector(&v.pos);
		for (ModelVertex &v: s->vertex)
			f->WriteInt(v.bone_index);

	    // skin vertices
		int num_skin_v = 0;
		for (int m=0;m<material.num;m++)
			num_skin_v += s->sub[m].triangle.num * material[m].num_textures * 3;
		f->WriteInt(num_skin_v);
		for (int m=0;m<material.num;m++)
			for (int tl=0;tl<material[m].num_textures;tl++)
		    	for (int j=0;j<s->sub[m].triangle.num;j++)
					for (int k=0;k<3;k++){
						f->WriteFloat(s->sub[m].triangle[j].skin_vertex[tl][k].x);
						f->WriteFloat(s->sub[m].triangle[j].skin_vertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<material.num;m++){
			ModelSubSkin *sub = &s->sub[m];

			// triangles
			f->WriteInt(sub->triangle.num);

			// vertex index
	    	for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++)
					f->WriteInt(sub->triangle[j].vertex[k]);

			// skin index
			for (int tl=0;tl<material[m].num_textures;tl++)
		    	for (int j=0;j<sub->triangle.num;j++)
					for (int k=0;k<3;k++)
						f->WriteInt(svi ++);

			// normal
	    	for (int j=0;j<sub->triangle.num;j++)
				for (int k=0;k<3;k++){
					sub->triangle[j].normal_index[k] = get_normal_index(sub->triangle[j].normal[k]);
					f->WriteWord(sub->triangle[j].normal_index[k]);
				}
			f->WriteInt(0);
		}

		f->WriteInt(0);
	}

// skeleton
	f->WriteComment("// Skeleton");
	f->WriteInt(bone.num);
	for (ModelBone &b: bone){
		if (b.parent >= 0){
			vector dpos = b.pos - bone[b.parent].pos;
			f->WriteVector(&dpos);
		}else
			f->WriteVector(&b.pos);
		f->WriteInt(b.parent);
		f->WriteStr(b.model_file);
	}

// animations
	f->WriteComment("// Animations");
	if ((move.num == 1) && (move[0].frame.num == 0)){
		f->WriteInt(0);
	}else
		f->WriteInt(move.num);
	int n_moves = 0;
	int n_frames_vert = 0;
	int n_frames_skel = 0;
	for (int i=0;i<move.num;i++)
		if (move[i].frame.num > 0){
			n_moves ++;
			if (move[i].type == MOVE_TYPE_VERTEX)	n_frames_vert += move[i].frame.num;
			if (move[i].type == MOVE_TYPE_SKELETAL)	n_frames_skel += move[i].frame.num;
		}
	f->WriteInt(n_moves);
	f->WriteInt(n_frames_vert);
	f->WriteInt(n_frames_skel);
	for (int i=0;i<move.num;i++)
		if (move[i].frame.num > 0){
			ModelMove *m = &move[i];
			bool rubber_timing = m->needsRubberTiming();
			f->WriteInt(i);
			f->WriteStr(m->name);
			f->WriteInt(m->type + (rubber_timing ? 128 : 0));
			f->WriteInt(m->frame.num);
			f->WriteFloat(m->frames_per_sec_const);
			f->WriteFloat(m->frames_per_sec_factor);

			// vertex animation
			if (m->type == MOVE_TYPE_VERTEX){
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->WriteFloat(fr.duration);
					for (int s=0;s<4;s++){
						// compress (only write != 0)
						int num_vertices = 0;
						for (int j=0;j<skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0)
								num_vertices ++;
						f->WriteInt(num_vertices);
						for (int j=0;j<skin[s].vertex.num;j++)
							if (fr.skin[i].dpos[j] != v_0){
								f->WriteInt(j);
								f->WriteVector(&fr.skin[i].dpos[j]);
							}
					}
				}
			// skeletal animation
			}else if (m->type == MOVE_TYPE_SKELETAL){
				for (int j=0;j<bone.num;j++)
					f->WriteBool((bone[j].parent < 0));
				f->WriteBool(m->interpolated_quadratic);
				f->WriteBool(m->interpolated_loop);
				for (ModelFrame &fr: m->frame){
					if (rubber_timing)
						f->WriteFloat(fr.duration);
					for (int j=0;j<bone.num;j++){
						f->WriteVector(&fr.skel_ang[j]);
						if (bone[j].parent < 0)
							f->WriteVector(&fr.skel_dpos[j]);
					}
				}
			}
		}

// effects
	f->WriteComment("// Effects");
	f->WriteInt(fx.num);
	for (int i=0;i<fx.num;i++){
		if (fx[i].type == FX_TYPE_SCRIPT){
			f->WriteStr("Script");
			f->WriteInt(fx[i].vertex);
			f->WriteStr(fx[i].file);
			f->WriteStr("");
		}else if (fx[i].type == FX_TYPE_LIGHT){
			f->WriteStr("Light");
			f->WriteInt(fx[i].vertex);
			f->WriteInt((int)fx[i].size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, fx[i].colors[nc]);
		}else if (fx[i].type == FX_TYPE_SOUND){
			f->WriteStr("Sound");
			f->WriteInt(fx[i].vertex);
			f->WriteInt((int)fx[i].size);
			f->WriteInt((int)(fx[i].speed * 100.0f));
			f->WriteStr(fx[i].file);
		}else if (fx[i].type == FX_TYPE_FORCEFIELD){
			f->WriteStr("ForceField");
			f->WriteInt(fx[i].vertex);
			f->WriteInt((int)fx[i].size);
			f->WriteInt((int)fx[i].intensity);
			f->WriteBool(fx[i].inv_quad);
		}
	}

// properties
	f->WriteComment("// Physics");
	f->WriteFloat(meta_data.mass);
	for (int i=0;i<9;i++)
		f->WriteFloat(meta_data.inertia_tensor.e[i]);
	f->WriteBool(meta_data.active_physics);
	f->WriteBool(meta_data.passive_physics);
	f->WriteFloat(radius);

	f->WriteComment("// LOD-Distances");
	f->WriteFloat(meta_data.detail_dist[0]);
	f->WriteFloat(meta_data.detail_dist[1]);
	f->WriteFloat(meta_data.detail_dist[2]);

// object data
	f->WriteComment("// Object Data");
	f->WriteStr(meta_data.name);
	f->WriteStr(meta_data.description);

	// inventary
	f->WriteComment("// Inventary");
	f->WriteInt(meta_data.inventary.num);
	for (int i=0;i<meta_data.inventary.num;i++){
	    f->WriteStr(meta_data.inventary[i]);
		f->WriteInt(1);
	}

	// script
	f->WriteComment("// Script");
	f->WriteStr(meta_data.script_file);
	f->WriteInt(meta_data.script_var.num);
	for (int i=0;i<meta_data.script_var.num;i++)
	    f->WriteFloat(meta_data.script_var[i]);

// additional data for editing
	f->WriteComment("// Editor");
	f->WriteBool(meta_data.auto_generate_tensor);
	f->WriteBool(meta_data.auto_generate_dists);
	f->WriteBool(meta_data.auto_generate_skin[1]);
	f->WriteBool(meta_data.auto_generate_skin[2]);
	f->WriteInt(meta_data.detail_factor[1]);
	f->WriteInt(meta_data.detail_factor[2]);
	f->WriteComment("// Normals");
	for (int i=1;i<4;i++){
		ModelSkin *s = &skin[i];
		f->WriteInt(NORMAL_MODE_PER_VERTEX);
		for (ModelVertex &v: s->vertex)
			f->WriteInt(v.normal_mode);
	}
	f->WriteComment("// Polygons");
	f->WriteInt(surface.num);
	for (ModelSurface &s: surface){
		f->WriteInt(s.polygon.num);
		for (ModelPolygon &t: s.polygon){
			f->WriteInt(t.side.num);
			f->WriteInt(t.material);
			for (ModelPolygonSide &ss: t.side){
				f->WriteInt(ss.vertex);
				for (int l=0;l<material[t.material].num_textures;l++){
					f->WriteFloat(ss.skin_vertex[l].x);
					f->WriteFloat(ss.skin_vertex[l].y);
				}
			}
		}
		f->WriteBool(s.is_physical);
		f->WriteBool(s.is_visible);
		f->WriteInt(0);
	}

	f->WriteComment("#");
	FileClose(f);

	action_manager->markCurrentAsSave();
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
			e.is_selected = (vertex[e.vertex[0]].is_selected && vertex[e.vertex[1]].is_selected);
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
	sv.add(e_y);
	sv.add(v_0);
	sv.add(e_x);
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
	msg_db_f("SurfJoin", 1);

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
	msg_db_f("GenerateDetailDists", 3);
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
	msg_db_f("GenerateInertiaTensor", 3);
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
		Matrix3Identity(t);

	return t;
}


int DataModel::getNumSelectedVertices()
{
	int r = 0;
	/*if ((CreationMode < 0) && ((SubMode == SubModeSkeleton) || ((SubMode == SubModeAnimation) && (move->Type == MoveTypeSkeletal)))){
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
			QuaternionRotationV(q0, f0.skel_ang[i]);
			QuaternionRotationV(q1, f1.skel_ang[i]);
			QuaternionInterpolate(q, q0, q1, t);
			f.skel_ang[i] = q.get_angles();
		}
	}
	return f;
}

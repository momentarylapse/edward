/*
 * DataModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "DataModel.h"
#include "ModeModelSurface.h"
#include "../../Mode/Model/ModeModel.h"
#include "../../Action/Action.h"
#include "../../Action/ActionManager.h"
#include "../../Edward.h"
#include "../../Action/Model/Mesh/Vertex/ActionModelAddVertex.h"
#include "../../Action/Model/Mesh/Triangle/ActionModelAddTriangleSingleTexture.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAddCube.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAddPlane.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAddCylinder.h"
#include "../../Action/Model/Mesh/Surface/ActionModelAddBall.h"




DataModel::DataModel()
{
	AutoTexturingData.enabled = false;

	ViewStage = 0; // TODO: mode...?
}

DataModel::~DataModel()
{
}


void DataModel::MetaData::Reset()
{
	// level of detail
	DetailDist[0] = 2000;
	DetailDist[1] = 4000;
	DetailDist[2] = 8000;
	AutoGenerateDists = true;
	DetailFactor[1] = 40;
	DetailFactor[2] = 20;
	AutoGenerateSkin[1] = false;//true;
	AutoGenerateSkin[2] = false;//true;

	// physics
	Mass = 1;
	ActivePhysics = true;
	PassivePhysics = true;
	AutoGenerateTensor = true;
	Matrix3Identity(InertiaTensor);

	// object data
	Name = "";
	Description = "";
	Inventary.clear();

	// script
	ScriptFile = "";
	ScriptVar.clear();
}



void DataModel::Reset()
{

	filename = "";
	for (int i=0;i<4;i++){
		Skin[i].Vertex.clear();
		Skin[i].NormalModeAll = NormalModeAngular;
		for (int j=0;j<Material.num;j++)
			Skin[i].Sub[j].Triangle.clear();
		Skin[i].Sub.resize(1);
	}
	Surface.clear();
	Vertex.clear();
	Ball.clear();
	Poly.clear();
	Fx.clear();
	Material.resize(1);
	Material[0].reset();
	NormalModeAll = NormalModeAngular;

	CurrentMaterial = 0;
	CurrentTextureLevel = 0;

	SkinVertMat = SkinVertTL = -1;

	// skeleton
	Bone.clear();

	// create one dummy animation
	Move.resize(1);
	move = &Move[0];
	CurrentMove = 0;
	CurrentFrame = 0;
	move->Frame.num = 0;
	move->Type = MoveTypeNone;
	move->FramesPerSecConst = 1;
	move->FramesPerSecFactor = 0;
	move->InterpolatedQuadratic = 0;
	move->InterpolatedLoop = false;
	move->Name = "";
	TimeScale = 1;
	TimeParam = 0;
	Playing = false;
	PlayLoop = true;


	for (int i=0;i<4;i++){
		Skin[i].Sub.resize(1);
		Skin[i].Sub[0].NumTextures = 1;
	}

	Radius = 42;

	meta_data.Reset();

	ResetHistory();
	Notify("Change");

//	Execute(new ActionModelAddCube(v0, e_x * 20, e_y * 20, e_z * 20, Vertex.num));
	/*Execute(new ActionModelAddPlane(e_y * 10, e_x * 200, e_z * 200, 4, 4, Vertex.num));
	Execute(new ActionModelAddBall(vector(-200, 0, 0), 100, 16, 16, false, Vertex.num));
	Execute(new ActionModelAddBall(vector(-200, 0, 200), 100, 16, 16, true, Vertex.num));
	Execute(new ActionModelAddCylinder(vector(200, 0, 0), e_z * 100, 40, 3, 16, true, Vertex.num));*/
}

void DataModel::DebugShow()
{
	msg_write("------------");
	msg_write(Vertex.num);
	msg_write(Surface.num);
	foreach(Surface, s){
		msg_write(s.Triangle.num);
		s.TestSanity("Model.DebugShow");
	}
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




bool DataModel::Load(const string & _filename, bool deep)
{
	msg_db_r("Model::LoadFromFile",1);
	//if (allow_load)
	Reset();
	bool error=false;
	int ffv;

	Array<vector> skin_vert;

	filename = _filename;
	if (this == mode_model->data)
		ed->MakeDirs(filename);
	//msg_write(dir);
	//msg_write(filename);
	CFile *f=new CFile();
	if (!f->Open(filename)){
		delete(f);
		ed->SetMessage(_("Datei ist nicht in der Stimmung, ge&offnet zu werden"));
		msg_db_l(1);
		return false;
	}
	file_time = f->GetDate(FileDateModification).time;

	ffv=f->ReadFileFormatVersion();

	if (ffv<0){
		ed->ErrorBox(_("Datei-Format unlesbar!"));
		msg_db_l(1);
		error=true;

	}else if (ffv==10){ // old format

		// Materials
		Material.resize(f->ReadIntC());
		for (int i=0;i<Material.num;i++){
			Material[i].MaterialFile = f->ReadStr();
			Material[i].UserColor = f->ReadBool();
			if (Material[i].UserColor){
				read_color_argb(f, Material[i].Ambient);
				read_color_argb(f, Material[i].Diffuse);
				read_color_argb(f, Material[i].Specular);
				read_color_argb(f, Material[i].Emission);
				Material[i].Shininess = (float)f->ReadInt();
			}
			Material[i].TransparencyMode = f->ReadInt();
			Material[i].UserTransparency = (Material[i].TransparencyMode != TransparencyModeDefault);
			Material[i].AlphaSource = f->ReadInt();
			Material[i].AlphaDestination = f->ReadInt();
			Material[i].AlphaFactor = (float)f->ReadInt() * 0.01f;
			Material[i].AlphaZBuffer = f->ReadBool();
			Material[i].NumTextures = f->ReadInt();
			for (int t=0;t<Material[i].NumTextures;t++)
				Material[i].TextureFile[t] = f->ReadStr();
		}
		// create subs...
		for (int k=0;k<4;k++){
			Skin[k].Sub.resize(Material.num);
			for (int j=0;j<Material.num;j++)
				Skin[k].Sub[j].NumTextures = 1;
		}

	// Physical Skin

		// vertices
		Skin[0].Vertex.resize(f->ReadIntC());
		for (int j=0;j<Skin[0].Vertex.num;j++){
			Skin[0].Vertex[j].BoneIndex = f->ReadInt();
			if (Skin[0].Vertex[j].BoneIndex < 0)
				Skin[0].Vertex[j].BoneIndex = 0;
			f->ReadVector(&Skin[0].Vertex[j].pos);
		}

		// triangles
		Skin[0].Sub[0].Triangle.resize(f->ReadInt());
		for (int j=0;j<Skin[0].Sub[0].Triangle.num;j++){
			Skin[0].Sub[0].Triangle[j].NormalDirty = true;
			for (int k=0;k<3;k++)
				Skin[0].Sub[0].Triangle[j].Vertex[k] = f->ReadInt();
		}

		// balls
		Ball.resize(f->ReadInt());
		for (int j=0;j<Ball.num;j++){
			Ball[j].Index = f->ReadInt();
			Ball[j].Radius = f->ReadFloat();
		}

		// polys
		Poly.resize(f->ReadInt());
		for (int j=0;j<Poly.num;j++){
			Poly[j].NumFaces = f->ReadInt();
			for (int k=0;k<Poly[j].NumFaces;k++){
				Poly[j].Face[k].NumVertices = f->ReadInt();
				for (int l=0;l<Poly[j].Face[k].NumVertices;l++)
					Poly[j].Face[k].Index[l] = f->ReadInt();
			}
		}

	// Skin[i]
		for (int i=1;i<4;i++){
			Skin[i].NormalModeAll = f->ReadIntC();
			bool pre_normals = (Skin[i].NormalModeAll & NormalModePre) > 0;
			Skin[i].NormalModeAll -= (Skin[i].NormalModeAll & NormalModePre);

			// vertices
			Skin[i].Vertex.resize(f->ReadInt());
			for (int j=0;j<Skin[i].Vertex.num;j++){
				Skin[i].Vertex[j].BoneIndex = f->ReadInt();
				if (Skin[i].Vertex[j].BoneIndex < 0)
					Skin[i].Vertex[j].BoneIndex = 0;
				f->ReadVector(&Skin[i].Vertex[j].pos);
				if (Skin[i].NormalModeAll == NormalModePerVertex)
					Skin[i].Vertex[j].NormalMode = f->ReadByte();
				else
					Skin[i].Vertex[j].NormalMode = Skin[i].NormalModeAll;
				Skin[i].Vertex[j].NormalDirty = true;
			}

			// skin vertices
			skin_vert.resize(f->ReadInt());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->ReadFloat();
				skin_vert[j].y = f->ReadFloat();
			}



			// triangles (subs)
			int num_trias = f->ReadInt();
			for (int t=0;t<Material.num;t++)
				Skin[i].Sub[t].Triangle.resize(f->ReadInt());
			for (int t=0;t<Material.num;t++)
				for (int j=0;j<Skin[i].Sub[t].Triangle.num;j++)
					for (int k=0;k<3;k++){
						Skin[i].Sub[t].Triangle[j].Vertex[k] = f->ReadInt();
						int svi = f->ReadInt();
						Skin[i].Sub[t].Triangle[j].SkinVertex[0][k] = skin_vert[svi];
						Skin[i].Sub[t].Triangle[j].NormalIndex[k] = (int)f->ReadByte();
						Skin[i].Sub[t].Triangle[j].NormalDirty = true;
					}
		}

	// Skeleton
		Bone.resize(f->ReadIntC());
		for (int i=0;i<Bone.num;i++){
			f->ReadVector(&Bone[i].DeltaPos);
			Bone[i].Parent = f->ReadInt();
			if (Bone[i].Parent > 32000)
				Bone[i].Parent = -1;
			Bone[i].ModelFile = f->ReadStr();
			if (deep)
				Bone[i].model = MetaLoadModel(Bone[i].ModelFile);
			Bone[i].ConstPos = false;
			Bone[i].is_selected = Bone[i].m_old = false;
		}

	// Animations
		int num_anims = f->ReadIntC();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->ReadInt();
			Move.resize(anim_index + 1);
			ModeModelMove *m = &Move[anim_index];
			m->Name = f->ReadStr();
			m->Type = f->ReadInt();
			m->Frame.resize(f->ReadInt());
			m->FramesPerSecConst = f->ReadFloat();
			m->FramesPerSecFactor = f->ReadFloat();

			// vertex animation
			if (m->Type == MoveTypeVertex){
				for (int fr=0;fr<m->Frame.num;fr++){
					for (int s=0;s<4;s++){
						m->Frame[fr].Skin[s].DPos.resize(Skin[s].Vertex.num);
						int num_vertices = f->ReadInt();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->ReadInt();
							f->ReadVector(&m->Frame[fr].Skin[s].DPos[vertex_index]);
						}
					}
				}
			}else if (m->Type == MoveTypeSkeletal){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(Bone.num);
				for (int j=0;j<Bone.num;j++)
					VarDeltaPos[j] = f->ReadBool();
				m->InterpolatedQuadratic = f->ReadBool();
				m->InterpolatedLoop = f->ReadBool();
				for (int fr=0;fr<m->Frame.num;fr++){
					m->Frame[fr].SkelDPos.resize(Bone.num);
					m->Frame[fr].SkelAng.resize(Bone.num);
					for (int j=0;j<Bone.num;j++){
						f->ReadVector(&m->Frame[fr].SkelAng[j]);
						if (VarDeltaPos[j])
							f->ReadVector(&m->Frame[fr].SkelDPos[j]);
					}
				}
			}
		}
		// Effects
		Fx.resize(f->ReadIntC());
		if (Fx.num>10000)
			Fx.clear();
		for (int i=0;i<Fx.num;i++){
			string fxkind = f->ReadStr();
			Fx[i].Kind = -1;
			if (fxkind == "Script"){
				Fx[i].Kind = FXKindScript;
				Fx[i].Vertex = f->ReadInt();
				Fx[i].File = f->ReadStr();
				Fx[i].Function = f->ReadStr();
			}
			if (fxkind == "Light"){
				Fx[i].Kind=FXKindLight;
				Fx[i].Vertex=f->ReadInt();
				Fx[i].Size=f->ReadInt();
				for (int j=0;j<3;j++)
					read_color_argb(f,Fx[i].Colors[j]);
			}
			if (fxkind == "Sound"){
				Fx[i].Kind=FXKindSound;
				Fx[i].Vertex=f->ReadInt();
				Fx[i].Size=f->ReadInt();
				Fx[i].Speed=f->ReadInt();
				Fx[i].File=f->ReadStr();
			}
			if (fxkind == "ForceField"){
				Fx[i].Kind=FXKindForceField;
				Fx[i].Vertex=f->ReadInt();
				Fx[i].Size=f->ReadInt();
				Fx[i].Intensity=f->ReadInt();
				Fx[i].InvQuad=f->ReadBool();
			}
			if (Fx[i].Kind<0)
				msg_error("unknown effekt: " + fxkind);
		}
		// LOD-Distances
		meta_data.DetailDist[0]=f->ReadFloatC();
		meta_data.DetailDist[1]=f->ReadFloat();
		meta_data.DetailDist[2]=f->ReadFloat();
		meta_data.AutoGenerateDists=f->ReadBool();
		meta_data.DetailFactor[1]=f->ReadByte();
		meta_data.DetailFactor[2]=f->ReadByte();
		meta_data.AutoGenerateSkin[1]=f->ReadBool();
		meta_data.AutoGenerateSkin[2]=f->ReadBool();
		// Physics
		meta_data.AutoGenerateTensor = f->ReadBoolC();
		for (int i=0;i<9;i++)
			meta_data.InertiaTensor.e[i] = f->ReadFloat();
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
		Material.resize(f->ReadIntC());
		for (int i=0;i<Material.num;i++){
			Material[i].MaterialFile = f->ReadStr();
			Material[i].UserColor = f->ReadBool();
			read_color_argb(f, Material[i].Ambient);
			read_color_argb(f, Material[i].Diffuse);
			read_color_argb(f, Material[i].Specular);
			read_color_argb(f, Material[i].Emission);
			Material[i].Shininess = (float)f->ReadInt();
			Material[i].TransparencyMode = f->ReadInt();
			Material[i].UserTransparency = (Material[i].TransparencyMode != TransparencyModeDefault);
			Material[i].AlphaSource = f->ReadInt();
			Material[i].AlphaDestination = f->ReadInt();
			Material[i].AlphaFactor = (float)f->ReadInt() * 0.01f;
			Material[i].AlphaZBuffer = f->ReadBool();
			Material[i].NumTextures = f->ReadInt();
			for (int t=0;t<Material[i].NumTextures;t++)
				Material[i].TextureFile[t] = f->ReadStr();
		}
		// create subs...
		for (int k=0;k<4;k++){
			Skin[k].Sub.resize(Material.num);
			for (int j=0;j<Material.num;j++)
				Skin[k].Sub[j].NumTextures = Material[j].NumTextures;
		}

	// Physical Skin

		// vertices
		Skin[0].Vertex.resize(f->ReadIntC());
		for (int j=0;j<Skin[0].Vertex.num;j++)
			Skin[0].Vertex[j].BoneIndex = f->ReadInt();
		for (int j=0;j<Skin[0].Vertex.num;j++)
			f->ReadVector(&Skin[0].Vertex[j].pos);

		// triangles
		f->ReadInt();

		// balls
		Ball.resize(f->ReadInt());
		for (int j=0;j<Ball.num;j++){
			Ball[j].Index = f->ReadInt();
			Ball[j].Radius = f->ReadFloat();
		}

		// polys
		Poly.resize(f->ReadInt());
		for (int j=0;j<Poly.num;j++){
			Poly[j].NumFaces = f->ReadInt();
			for (int k=0;k<Poly[j].NumFaces;k++){
				Poly[j].Face[k].NumVertices = f->ReadInt();
				for (int l=0;l<Poly[j].Face[k].NumVertices;l++)
					Poly[j].Face[k].Index[l] = f->ReadInt();
				f->ReadFloat();
				f->ReadFloat();
				f->ReadFloat();
				f->ReadFloat();
			}
			Poly[j].NumSVertices = f->ReadInt();
			for (int k=0;k<Poly[j].NumSVertices;k++)
				f->ReadInt();
			Poly[j].NumEdges = f->ReadInt();
			for (int k=0;k<Poly[j].NumEdges*2;k++)
				f->ReadInt();
			// topology
			for (int k=0;k<Poly[j].NumFaces*Poly[j].NumFaces;k++)
				f->ReadInt();
			for (int k=0;k<Poly[j].NumEdges*Poly[j].NumFaces;k++)
				f->ReadBool();
		}

	// Skin[i]
		for (int i=1;i<4;i++){

			// vertices
			Skin[i].Vertex.resize(f->ReadIntC());
			for (int j=0;j<Skin[i].Vertex.num;j++)
				f->ReadVector(&Skin[i].Vertex[j].pos);
			for (int j=0;j<Skin[i].Vertex.num;j++)
				Skin[i].Vertex[j].BoneIndex = f->ReadInt();
			for (int j=0;j<Skin[i].Vertex.num;j++)
				Skin[i].Vertex[j].NormalDirty = false;//true;

			// skin vertices
			skin_vert.resize(f->ReadInt());
			for (int j=0;j<skin_vert.num;j++){
				skin_vert[j].x = f->ReadFloat();
				skin_vert[j].y = f->ReadFloat();
			}



			// triangles (subs)
			for (int m=0;m<Material.num;m++){
				Skin[i].Sub[m].Triangle.resize(f->ReadInt());
				// vertex
				for (int j=0;j<Skin[i].Sub[m].Triangle.num;j++)
					for (int k=0;k<3;k++)
						Skin[i].Sub[m].Triangle[j].Vertex[k] = f->ReadInt();
				// skin vertex
				for (int tl=0;tl<Material[m].NumTextures;tl++)
					for (int j=0;j<Skin[i].Sub[m].Triangle.num;j++)
						for (int k=0;k<3;k++){
							int svi = f->ReadInt();
							Skin[i].Sub[m].Triangle[j].SkinVertex[tl][k] = skin_vert[svi];
						}
				// normals
				for (int j=0;j<Skin[i].Sub[m].Triangle.num;j++){
					for (int k=0;k<3;k++){
						Skin[i].Sub[m].Triangle[j].NormalIndex[k] = (int)(unsigned short)f->ReadWord();
						Skin[i].Sub[m].Triangle[j].Normal[k] = get_normal_by_index(Skin[i].Sub[m].Triangle[j].NormalIndex[k]);
					}
					Skin[i].Sub[m].Triangle[j].NormalDirty = false;
				}
				f->ReadInt();
			}
			f->ReadInt();
		}

	// Skeleton
		Bone.resize(f->ReadIntC());
		for (int i=0;i<Bone.num;i++){
			f->ReadVector(&Bone[i].DeltaPos);
			Bone[i].Parent = f->ReadInt();
			Bone[i].ModelFile = f->ReadStr();
			if (deep)
				Bone[i].model = MetaLoadModel(Bone[i].ModelFile);
			Bone[i].ConstPos = false;
			Bone[i].is_selected = Bone[i].m_old = false;
		}

	// Animations
		Move.resize(f->ReadIntC());
		int num_anims = f->ReadInt();
		f->ReadInt();
		f->ReadInt();
		for (int i=0;i<num_anims;i++){
			int anim_index = f->ReadInt();
			Move.resize(anim_index + 1);
			ModeModelMove *m = &Move[anim_index];
			m->Name = f->ReadStr();
			m->Type = f->ReadInt();
			m->Frame.resize(f->ReadInt());
			m->FramesPerSecConst = f->ReadFloat();
			m->FramesPerSecFactor = f->ReadFloat();

			// vertex animation
			if (m->Type == MoveTypeVertex){
				for (int fr=0;fr<m->Frame.num;fr++){
					for (int s=0;s<4;s++){
						m->Frame[fr].Skin[s].DPos.resize(Skin[s].Vertex.num);
						int num_vertices = f->ReadInt();
						for (int j=0;j<num_vertices;j++){
							int vertex_index = f->ReadInt();
							f->ReadVector(&m->Frame[fr].Skin[s].DPos[vertex_index]);
						}
					}
				}
			}else if (m->Type == MoveTypeSkeletal){
				Array<bool> VarDeltaPos;
				VarDeltaPos.resize(Bone.num);
				for (int j=0;j<Bone.num;j++)
					VarDeltaPos[j] = f->ReadBool();
				m->InterpolatedQuadratic = f->ReadBool();
				m->InterpolatedLoop = f->ReadBool();
				for (int fr=0;fr<m->Frame.num;fr++){
					m->Frame[fr].SkelDPos.resize(Bone.num);
					m->Frame[fr].SkelAng.resize(Bone.num);
					for (int j=0;j<Bone.num;j++){
						f->ReadVector(&m->Frame[fr].SkelAng[j]);
						if (VarDeltaPos[j])
							f->ReadVector(&m->Frame[fr].SkelDPos[j]);
					}
				}
			}
		}
		// Effects
		Fx.resize(f->ReadIntC());
		if (Fx.num>10000)
			Fx.clear();
		for (int i=0;i<Fx.num;i++){
			string fxkind = f->ReadStr();
			Fx[i].Kind=-1;
			if (fxkind == "Script"){
				Fx[i].Kind = FXKindScript;
				Fx[i].Vertex = f->ReadInt();
				Fx[i].File = f->ReadStr();
				Fx[i].Function = f->ReadStr();
			}
			if (fxkind == "Light"){
				Fx[i].Kind=FXKindLight;
				Fx[i].Vertex=f->ReadInt();
				Fx[i].Size=f->ReadInt();
				for (int j=0;j<3;j++)
					read_color_argb(f,Fx[i].Colors[j]);
			}
			if (fxkind == "Sound"){
				Fx[i].Kind = FXKindSound;
				Fx[i].Vertex = f->ReadInt();
				Fx[i].Size = f->ReadInt();
				Fx[i].Speed = f->ReadInt();
				Fx[i].File = f->ReadStr();
			}
			if (fxkind == "ForceField"){
				Fx[i].Kind = FXKindForceField;
				Fx[i].Vertex = f->ReadInt();
				Fx[i].Size = f->ReadInt();
				Fx[i].Intensity = f->ReadInt();
				Fx[i].InvQuad = f->ReadBool();
			}
			if (Fx[i].Kind<0)
				msg_error("unknown effekt: " + fxkind);
		}

// properties
		// Physics
		meta_data.Mass = f->ReadFloatC();
		for (int i=0;i<9;i++)
			meta_data.InertiaTensor.e[i] = f->ReadFloat();
		meta_data.ActivePhysics = f->ReadBool();
		meta_data.PassivePhysics = f->ReadBool();
		Radius = f->ReadFloat();

		// LOD-Distances
		meta_data.DetailDist[0] = f->ReadFloatC();
		meta_data.DetailDist[1] = f->ReadFloat();
		meta_data.DetailDist[2] = f->ReadFloat();

// object data
		// Object Data
		meta_data.Name = f->ReadStrC();
		meta_data.Description = f->ReadStr();

		// Inventary
		meta_data.Inventary.resize(f->ReadIntC());
		for (int i=0;i<meta_data.Inventary.num;i++){
			meta_data.Inventary[i] = f->ReadStr();
			f->ReadInt();
		}

		// Script
		meta_data.ScriptFile = f->ReadStrC();
		meta_data.ScriptVar.resize(f->ReadInt());
		for (int i=0;i<meta_data.ScriptVar.num;i++)
			meta_data.ScriptVar[i] = f->ReadFloat();


// additional data for editing
		// Editor
		meta_data.AutoGenerateTensor = f->ReadBoolC();
		meta_data.AutoGenerateDists = f->ReadBool();
		meta_data.AutoGenerateSkin[1] = f->ReadBool();
		meta_data.AutoGenerateSkin[2] = f->ReadBool();
		meta_data.DetailFactor[1] = f->ReadInt();
		meta_data.DetailFactor[2] = f->ReadInt();
		// Normals
		f->ReadComment();
		for (int i=1;i<4;i++){
			ModeModelSkin *s = &Skin[i];
			s->NormalModeAll = f->ReadInt();
			if (s->NormalModeAll == NormalModePerVertex){
				foreach(s->Vertex, v)
					v.NormalMode = f->ReadInt();
			}else{
				foreach(s->Vertex, v)
					v.NormalMode = s->NormalModeAll;
			}
		   }
		// BG Textures
		f->ReadComment();
	/*for (int i=0;i<4;i++){
		f->WriteStr(BgTextureFile[i]);
		if (strlen(BgTextureFile[i])>0){
			f->WriteFloat(BgTextureA[i].x);
			f->WriteFloat(BgTextureA[i].y);
			f->WriteFloat(BgTextureA[i].z);
			f->WriteFloat(BgTextureB[i].x);
			f->WriteFloat(BgTextureB[i].y);
			f->WriteFloat(BgTextureB[i].z);
		}
	}*/





	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 10, 10));
		error=true;
	}

	f->Close();
	delete(f);




		// import...
	NotifyBegin();
	NormalModeAll = Skin[1].NormalModeAll;
		foreach(Skin[1].Vertex, v)
			AddVertex(v.pos);
		for (int i=0;i<Material.num;i++){
			CurrentMaterial = i;
			foreach(Skin[1].Sub[i].Triangle, t){
				ModeModelTriangle *tt = AddTriangle(t.Vertex[0], t.Vertex[1], t.Vertex[2]);
				for (int tl=0;tl<Material[i].NumTextures;tl++)
					for (int k=0;k<3;k++)
						tt->SkinVertex[tl][k] = t.SkinVertex[tl][k];
			}
		}
	ClearSelection();
	NotifyEnd();



	if (deep){
		for (int i=0;i<Material.num;i++){
			Material[i].MakeConsistent();

			// test textures
			for (int t=0;t<Material[i].NumTextures;t++){
				if ((Material[i].Texture[t] < 0) && (Material[i].TextureFile[t].num > 0))
					ed->SetMessage(format(_("Textur-Datei nicht ladbar: %s"), Material[i].TextureFile[t].c_str()));
			}
		}



		// TODO -> mode...
		/*if (this == mode_model->data){
			ed->SetTitle(filename);
			ResetView();
		}*/
	}


	//OptimizeView();
	ResetHistory();

	UpdateNormals();
	msg_db_l(1);
	return !error;
}


void DataModel::GetBoundingBox(vector &min, vector &max)
{
	// bounding box (visual skin[1])
	min = max = v0;
	for (int i=0;i<Skin[1].Vertex.num;i++){
		VecMin(min, Skin[1].Vertex[i].pos);
		VecMax(max, Skin[1].Vertex[i].pos);
	}
	// (physical skin)
	for (int i=0;i<Skin[0].Vertex.num;i++){
		VecMin(min, Skin[0].Vertex[i].pos);
		VecMax(max, Skin[0].Vertex[i].pos);
	}
	for (int i=0;i<Ball.num;i++){
		VecMin(min, Skin[0].Vertex[Ball[i].Index].pos - vector(1,1,1) * Ball[i].Radius);
		VecMax(max, Skin[0].Vertex[Ball[i].Index].pos + vector(1,1,1) * Ball[i].Radius);
	}
}


bool DataModel::Save(const string & _filename)
{
	msg_db_r("DataModel.Save",1);

	/*if (AutoGenerateSkin[1])
		CreateSkin(&Skin[1],&Skin[2],(float)DetailFactor[1]*0.01f);

	if (AutoGenerateSkin[2])
		CreateSkin(&Skin[2],&Skin[3],(float)DetailFactor[2]/(float)DetailFactor[1]);*/

	if (meta_data.AutoGenerateDists)
		GenerateDetailDists(meta_data.DetailDist);

	if (meta_data.AutoGenerateTensor)
		meta_data.InertiaTensor = GenerateInertiaTensor(meta_data.Mass);



	// export...
	Skin[1].NormalModeAll = NormalModeAll;
	Skin[1].Vertex = Vertex;
	Skin[1].Sub.clear();
	Skin[1].Sub.resize(Material.num);
	foreach(Surface, s)
		foreach(s.Triangle, t)
			Skin[1].Sub[t.Material].Triangle.add(t);
	foreachi(Material, m, i)
		Skin[1].Sub[i].NumTextures = m.NumTextures;



#ifdef FORCE_UPDATE_NORMALS
	for (int d=1;d<4;d++)
		for (int j=0;j<Skin[d].NumVertices;j++)
			Skin[d].Vertex[j].NormalDirty = true;
#endif
	UpdateNormals();

//	PrecreatePhysicalData();

	GetBoundingBox(Min, Max);
	GetDiameter();
	Radius = GetDiameter() * 0.6f;


	// so the materials don't get mixed up
//	RemoveUnusedData();

	filename = _filename;
	ed->MakeDirs(filename);

	CFile *f = CreateFile(filename);
	f->WriteFileFormatVersion(false, 11);//FFVBinary, 11);
	f->FloatDecimals = 5;

// general
	f->WriteComment("// General");
	f->WriteVector(&Min);
	f->WriteVector(&Max);
	f->WriteInt(3); // skins...
	f->WriteInt(0); // reserved
	f->WriteInt(0);
	f->WriteInt(0);

// materials
	f->WriteComment("// Materials");
	f->WriteInt(Material.num);
	foreach(Material, m){
		f->WriteStr(m.MaterialFile);
		f->WriteBool(m.UserColor);
		write_color_argb(f, m.Ambient);
		write_color_argb(f, m.Diffuse);
		write_color_argb(f, m.Specular);
		write_color_argb(f, m.Emission);
		f->WriteInt(m.Shininess);
		f->WriteInt(m.UserTransparency ? m.TransparencyMode : TransparencyModeDefault);
		f->WriteInt(m.AlphaSource);
		f->WriteInt(m.AlphaDestination);
		f->WriteInt(m.AlphaFactor * 100.0f);
		f->WriteBool(m.AlphaZBuffer);
		f->WriteInt(m.NumTextures);
		for (int t=0;t<m.NumTextures;t++)
			f->WriteStr(m.TextureFile[t]);
	}

// physical skin
	f->WriteComment("// Physical Skin");

	// vertices
	f->WriteInt(Skin[0].Vertex.num);
	for (int j=0;j<Skin[0].Vertex.num;j++)
		f->WriteInt(Skin[0].Vertex[j].BoneIndex);
	for (int j=0;j<Skin[0].Vertex.num;j++)
		f->WriteVector(&Skin[0].Vertex[j].pos);

	// triangles
	f->WriteInt(0);
	/*for (int j=0;j<Skin[0].NumTriangles;j++)
		for (int k=0;k<3;k++)
			f->WriteInt(Skin[0].Triangle[j].Index[k]);*/

	// balls
	f->WriteInt(Ball.num);
	for (int j=0;j<Ball.num;j++){
		f->WriteInt(Ball[j].Index);
		f->WriteFloat(Ball[j].Radius);
	}

	f->WriteInt(Poly.num);
	for (int j=0;j<Poly.num;j++){
		f->WriteInt(Poly[j].NumFaces);
		for (int k=0;k<Poly[j].NumFaces;k++){
			f->WriteInt(Poly[j].Face[k].NumVertices);
			for (int l=0;l<Poly[j].Face[k].NumVertices;l++)
				f->WriteInt(Poly[j].Face[k].Index[l]);
			f->WriteFloat(Poly[j].Face[k].Plane.a);
			f->WriteFloat(Poly[j].Face[k].Plane.b);
			f->WriteFloat(Poly[j].Face[k].Plane.c);
			f->WriteFloat(Poly[j].Face[k].Plane.d);
		}
		f->WriteInt(Poly[j].NumSVertices);
		for (int k=0;k<Poly[j].NumSVertices;k++)
			f->WriteInt(Poly[j].SIndex[k]);
		f->WriteInt(Poly[j].NumEdges);
		for (int k=0;k<Poly[j].NumEdges;k++){
			f->WriteInt(Poly[j].EdgeIndex[k*2 + 0]);
			f->WriteInt(Poly[j].EdgeIndex[k*2 + 1]);
		}
		// topology
		for (int k=0;k<Poly[j].NumFaces;k++)
			for (int l=0;l<Poly[j].NumFaces;l++)
				f->WriteInt(Poly[j].FacesJoiningEdge[k * Poly[j].NumFaces + l]);
		for (int k=0;k<Poly[j].NumEdges;k++)
			for (int l=0;l<Poly[j].NumFaces;l++)
			    f->WriteBool(Poly[j].EdgeOnFace[k * Poly[j].NumFaces + l]);
	}

// skin
	for (int i=1;i<4;i++){
		ModeModelSkin *s = &Skin[i];
		f->WriteComment(format("// Skin[%d]",i));

		// verices
		f->WriteInt(s->Vertex.num);
		foreach(s->Vertex, v)
			f->WriteVector(&v.pos);
		foreach(s->Vertex, v)
			f->WriteInt(v.BoneIndex);

	    // skin vertices
		int num_skin_v = 0;
		for (int m=0;m<Material.num;m++)
			num_skin_v += s->Sub[m].Triangle.num * s->Sub[m].NumTextures * 3;
		f->WriteInt(num_skin_v);
		for (int m=0;m<Material.num;m++)
			for (int tl=0;tl<s->Sub[m].NumTextures;tl++)
		    	for (int j=0;j<s->Sub[m].Triangle.num;j++)
					for (int k=0;k<3;k++){
						f->WriteFloat(s->Sub[m].Triangle[j].SkinVertex[tl][k].x);
						f->WriteFloat(s->Sub[m].Triangle[j].SkinVertex[tl][k].y);
					}


		// sub skins
		int svi = 0;
		for (int m=0;m<Material.num;m++){
			ModeModelSubSkin *sub = &s->Sub[m];

			// triangles
			f->WriteInt(sub->Triangle.num);

			// vertex index
	    	for (int j=0;j<sub->Triangle.num;j++)
				for (int k=0;k<3;k++)
					f->WriteInt(sub->Triangle[j].Vertex[k]);

			// skin index
			for (int tl=0;tl<sub->NumTextures;tl++)
		    	for (int j=0;j<sub->Triangle.num;j++)
					for (int k=0;k<3;k++)
						f->WriteInt(svi ++);

			// normal
	    	for (int j=0;j<sub->Triangle.num;j++)
				for (int k=0;k<3;k++){
					sub->Triangle[j].NormalIndex[k] = get_normal_index(sub->Triangle[j].Normal[k]);
					f->WriteWord(sub->Triangle[j].NormalIndex[k]);
				}
			f->WriteInt(0);
		}

		f->WriteInt(0);
	}

// skeleton
	f->WriteComment("// Skeleton");
	f->WriteInt(Bone.num);
	for (int i=0;i<Bone.num;i++){
		f->WriteVector(&Bone[i].DeltaPos);
		f->WriteInt(Bone[i].Parent);
		f->WriteStr(Bone[i].ModelFile);
	}

// animations
	f->WriteComment("// Animations");
	if ((Move.num == 1) && (Move[0].Frame.num == 0)){
		f->WriteInt(0);
	}else
		f->WriteInt(Move.num);
	int n_moves = 0;
	int n_frames_vert = 0;
	int n_frames_skel = 0;
	for (int i=0;i<Move.num;i++)
		if (Move[i].Frame.num > 0){
			n_moves ++;
			if (Move[i].Type == MoveTypeVertex)	n_frames_vert += Move[i].Frame.num;
			if (Move[i].Type == MoveTypeSkeletal)	n_frames_skel += Move[i].Frame.num;
		}
	f->WriteInt(n_moves);
	f->WriteInt(n_frames_vert);
	f->WriteInt(n_frames_skel);
	for (int i=0;i<Move.num;i++)
		if (Move[i].Frame.num > 0){
			ModeModelMove *m = &Move[i];
			f->WriteInt(i);
			f->WriteStr(m->Name);
			f->WriteInt(m->Type);
			f->WriteInt(m->Frame.num);
			f->WriteFloat(m->FramesPerSecConst);
			f->WriteFloat(m->FramesPerSecFactor);

			// vertex animation
			if (m->Type == MoveTypeVertex){
				for (int fr=0;fr<m->Frame.num;fr++){
					for (int s=0;s<4;s++){
						// compress (only write != 0)
						int num_vertices = 0;
						for (int j=0;j<Skin[s].Vertex.num;j++)
							if (m->Frame[fr].Skin[i].DPos[j] != v0)
								num_vertices ++;
						f->WriteInt(num_vertices);
						for (int j=0;j<Skin[s].Vertex.num;j++)
							if (m->Frame[fr].Skin[i].DPos[j] != v0){
								f->WriteInt(j);
								f->WriteVector(&m->Frame[fr].Skin[i].DPos[j]);
							}
					}
				}
			// skeletal animation
			}else if (m->Type == MoveTypeSkeletal){
				for (int j=0;j<Bone.num;j++)
					f->WriteBool((Bone[j].Parent < 0));
				f->WriteBool(m->InterpolatedQuadratic);
				f->WriteBool(m->InterpolatedLoop);
				for (int fr=0;fr<m->Frame.num;fr++)
					for (int j=0;j<Bone.num;j++){
						f->WriteVector(&m->Frame[fr].SkelAng[j]);
						if (Bone[j].Parent < 0)
							f->WriteVector(&m->Frame[fr].SkelDPos[j]);
					}
			}
		}

// effects
	f->WriteComment("// Effects");
	f->WriteInt(Fx.num);
	for (int i=0;i<Fx.num;i++){
		if (Fx[i].Kind==FXKindScript){
			f->WriteStr("Script");
			f->WriteInt(Fx[i].Vertex);
			f->WriteStr(Fx[i].File);
			f->WriteStr(Fx[i].Function);
		}else if (Fx[i].Kind==FXKindLight){
			f->WriteStr("Light");
			f->WriteInt(Fx[i].Vertex);
			f->WriteInt(Fx[i].Size);
			for (int nc=0;nc<3;nc++)
				write_color_argb(f, Fx[i].Colors[nc]);
		}else if (Fx[i].Kind==FXKindSound){
			f->WriteStr("Sound");
			f->WriteInt(Fx[i].Vertex);
			f->WriteInt(Fx[i].Size);
			f->WriteInt(Fx[i].Speed);
			f->WriteStr(Fx[i].File);
		}else if (Fx[i].Kind==FXKindForceField){
			f->WriteStr("ForceField");
			f->WriteInt(Fx[i].Vertex);
			f->WriteInt(Fx[i].Size);
			f->WriteInt(Fx[i].Intensity);
			f->WriteBool(Fx[i].InvQuad);
		}
	}

// properties
	f->WriteComment("// Physics");
	f->WriteFloat(meta_data.Mass);
	for (int i=0;i<9;i++)
		f->WriteFloat(meta_data.InertiaTensor.e[i]);
	f->WriteBool(meta_data.ActivePhysics);
	f->WriteBool(meta_data.PassivePhysics);
	f->WriteFloat(Radius);

	f->WriteComment("// LOD-Distances");
	f->WriteFloat(meta_data.DetailDist[0]);
	f->WriteFloat(meta_data.DetailDist[1]);
	f->WriteFloat(meta_data.DetailDist[2]);

// object data
	f->WriteComment("// Object Data");
	f->WriteStr(meta_data.Name);
	f->WriteStr(meta_data.Description);

	// inventary
	f->WriteComment("// Inventary");
	f->WriteInt(meta_data.Inventary.num);
	for (int i=0;i<meta_data.Inventary.num;i++){
	    f->WriteStr(meta_data.Inventary[i]);
		f->WriteInt(1);
	}

	// script
	f->WriteComment("// Script");
	f->WriteStr(meta_data.ScriptFile);
	f->WriteInt(meta_data.ScriptVar.num);
	for (int i=0;i<meta_data.ScriptVar.num;i++)
	    f->WriteFloat(meta_data.ScriptVar[i]);

// additional data for editing
	f->WriteComment("// Editor");
	f->WriteBool(meta_data.AutoGenerateTensor);
	f->WriteBool(meta_data.AutoGenerateDists);
	f->WriteBool(meta_data.AutoGenerateSkin[1]);
	f->WriteBool(meta_data.AutoGenerateSkin[2]);
	f->WriteInt(meta_data.DetailFactor[1]);
	f->WriteInt(meta_data.DetailFactor[2]);
	f->WriteComment("// Normals");
	for (int i=1;i<4;i++){
		ModeModelSkin *s = &Skin[i];
		f->WriteInt(s->NormalModeAll);
		if (s->NormalModeAll == NormalModePerVertex)
			foreach(s->Vertex, v)
				f->WriteInt(v.NormalMode);
	   }
	/*f->WriteComment("// BG Textures");
	for (int i=0;i<4;i++){
		f->WriteStr(BgTextureFile[i]);
		if (BgTextureFile[i].num > 0){
			f->WriteFloat(BgTextureA[i].x);
			f->WriteFloat(BgTextureA[i].y);
			f->WriteFloat(BgTextureA[i].z);
			f->WriteFloat(BgTextureB[i].x);
			f->WriteFloat(BgTextureB[i].y);
			f->WriteFloat(BgTextureB[i].z);
		}
	}*/

	f->WriteComment("#");
	FileClose(f);

	ed->SetMessage(_("Gespeichert!"));
	action_manager->MarkCurrentAsSave();
	msg_db_l(1);
	return true;
}

void DataModel::SetNormalsDirtyByVertices(const Array<int> &index)
{
	foreach(Surface, s)
		foreach(s.Triangle, t)
			for (int k=0;k<3;k++)
				if (!t.NormalDirty)
					for (int i=0;i<index.num;i++)
						if (t.Vertex[k] == index[i]){
							t.NormalDirty = true;
							break;
						}
}

void DataModel::SetAllNormalsDirty()
{
	foreach(Surface, s)
		foreach(s.Triangle, t)
			t.NormalDirty = true;
}


void DataModel::UpdateNormals()
{
	foreach(Surface, s)
		s.UpdateNormals();
}

ModeModelSurface *DataModel::AddSurface(int surf_no)
{
	ModeModelSurface s;
	s.model = this;
	s.view_stage = ViewStage;
	s.is_selected = true;
	s.IsClosed = false;
	s.IsVisible = true;
	s.IsPhysical = true;
	if (surf_no >= 0){
		Surface.insert(s, surf_no);
		return &Surface[surf_no];
	}else{
		Surface.add(s);
		return &Surface.back();
	}
}


void DataModel::AddVertex(const vector &v, int normal_mode)
{	Execute(new ActionModelAddVertex(v, normal_mode));	}

void DataModel::ClearSelection()
{
	foreach(Vertex, v)
		v.is_selected = false;
	foreach(Surface, s){
		s.is_selected = false;
		foreach(s.Triangle, t)
			t.is_selected = false;
	}
}

void DataModel::SelectionTrianglesFromSurfaces()
{
	foreach(Surface, s)
		foreach(s.Triangle, t)
			t.is_selected = s.is_selected;
}

void DataModel::SelectionVerticesFromSurfaces()
{
	foreach(Vertex, v)
		if (v.Surface >= 0)
			v.is_selected = Surface[v.Surface].is_selected;
		else
			v.is_selected = false;
}

void DataModel::SelectionSurfacesFromTriangles()
{
	foreach(Surface, s){
		s.is_selected = true;
		foreach(s.Triangle, t)
			s.is_selected &= t.is_selected;
	}
}

void DataModel::SelectionVerticesFromTriangles()
{
	foreach(Vertex, v)
		v.is_selected = false;
	foreach(Surface, s)
		foreach(s.Triangle, t)
			if (t.is_selected)
				for (int k=0;k<3;k++)
					Vertex[t.Vertex[k]].is_selected = true;
}

void DataModel::SelectionTrianglesFromVertices()
{
	foreach(Surface, s)
		foreach(s.Triangle, t)
			t.is_selected = ((Vertex[t.Vertex[0]].is_selected) and (Vertex[t.Vertex[1]].is_selected) and (Vertex[t.Vertex[2]].is_selected));
}

ModeModelTriangle *DataModel::AddTriangle(int a, int b, int c)
{
	vector sv[3] = {e_y, v0, e_x};
	//ApplyAutoTexturing(this, a, b, c, sv);
	return (ModeModelTriangle*) Execute(new ActionModelAddTriangleSingleTexture(this, a, b, c, CurrentMaterial, sv[0], sv[1], sv[2]));
}



int DataModel::get_surf_no(ModeModelSurface *s)
{
	foreachi(Surface, ss, i)
		if (&ss == s)
			return i;
	return -1;
}

ModeModelSurface *DataModel::SurfaceJoin(ModeModelSurface *a, ModeModelSurface *b)
{
	msg_db_r("SurfJoin", 1);

	a->TestSanity("Join prae a");
	b->TestSanity("Join prae b");

	int ai = get_surf_no(a);
	int bi = get_surf_no(b);

	// correct edge data of b
	foreach(b->Edge, e){
		if (e.Triangle[0] >= 0)
			e.Triangle[0] += a->Triangle.num;
		if (e.Triangle[1] >= 0)
			e.Triangle[1] += a->Triangle.num;
	}

	// correct triangle data of b
	foreach(b->Triangle, t)
		for (int k=0;k<3;k++)
			t.Edge[k] += a->Edge.num;

	// correct vertex data of b
	foreach(b->Vertex, v)
		Vertex[v].Surface = ai;

	// insert data
	a->Vertex.join(b->Vertex);
	a->Edge.append(b->Edge);
	a->Triangle.append(b->Triangle);

	// remove surface
	if (bi >= 0)
		Surface.erase(bi);
	a = &Surface[ai];
	a->TestSanity("Join post a");

	msg_db_l(1);
	return a;
}

void DataModel::ResetAutoTexturing()
{
	if (AutoTexturingData.enabled)
		CurrentMaterial = AutoTexturingData.prev_material;
	AutoTexturingData.enabled = false;
}

void DataModel::ApplyAutoTexturing(int a, int b, int c, vector *sv)
{
	if (AutoTexturingData.enabled){
		vector v[3];
		v[0] = Vertex[a].pos;
		v[1] = Vertex[b].pos;
		v[2] = Vertex[c].pos;
		for (int k=0;k<3;k++)
			sv[k] = vector( (v[k] - AutoTexturingData.p0) * AutoTexturingData.dir_u, (v[k] - AutoTexturingData.p0) * AutoTexturingData.dir_v, 0);
	}
}

void DataModel::CreateSkin(ModeModelSkin *src, ModeModelSkin *dst, float quality_factor)
{
	msg_todo("DataModel::CreateSkin");
}




float DataModel::GetDiameter()
{
	float Diameter=0;
	foreach(Vertex, v){
		float d = VecLength(v.pos) * 2;
		if (d > Diameter)
			Diameter = d;
	}
			/*for (int p2=p1+1;p2<Skin[m].Vertex.num;p2++){
				float d=VecLength(GetVertex(&Skin[m],p1)-GetVertex(&Skin[m],p2));
				if (d>Diameter)
					Diameter=d;
			}*/
	return Diameter;
}

float DetailDistTemp1,DetailDistTemp2,DetailDistTemp3;

int get_num_trias(DataModel *m, ModeModelSkin *s)
{
	int n = 0;
	for (int i=0;i<m->Material.num;i++)
		n += s->Sub[i].Triangle.num;
	return n;
}

void DataModel::GenerateDetailDists(float *dist)
{
	msg_db_r("GenerateDetailDists", 3);
	float Diameter = GetDiameter();
	dist[0] = Diameter * 5;
	dist[1] = Diameter * 20;
	dist[2] = Diameter * 40;
	if (get_num_trias(this, &Skin[3]) == 0)
		dist[1] = dist[2];
	if (get_num_trias(this, &Skin[2]) == 0)
		dist[0] = dist[1];
	msg_db_l(3);
}


#define n_theta		16

matrix3 DataModel::GenerateInertiaTensor(float mass)
{
	msg_db_r("GenerateInertiaTensor", 3);
//	sModeModelSkin *p = &Skin[0];

	// estimate size
	vector min = v0, max = v0;
	foreach(Vertex, v){
		VecMin(min, v.pos);
		VecMax(max, v.pos);
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
				foreach(Surface, s)
					if (s.IsInside(r)){
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

	if (num_ds>0){
		float f = mass / num_ds;
		t *= f;
		t._10 = t._01;
		t._21 = t._12;
		t._02 = t._20;
	}else
		Matrix3Identity(t);

	msg_db_l(3);
	return t;
}


int DataModel::GetNumMarkedVertices()
{
	int r = 0;
	/*if ((CreationMode < 0) && ((SubMode == SubModeSkeleton) || ((SubMode == SubModeAnimation) && (move->Type == MoveTypeSkeletal)))){
		for (int i=0;i<Bone.num;i++)
			if (Bone[i].IsSelected)
				r++;
		return r;
	}*/
	foreach(Vertex, v)
		if (v.is_selected)
			r ++;
	return r;
}

int DataModel::GetNumMarkedSkinVertices()
{
	int r=0;
	return r;
}

int DataModel::GetNumMarkedTriangles()
{
	int r = 0;
	foreach(Surface, s)
		foreach(s.Triangle, t)
			if (t.is_selected)
				r ++;
	return r;
}

int DataModel::GetNumMarkedSurfaces()
{
	int r = 0;
	foreach(Surface, s)
		if (s.is_selected)
			r ++;
	return r;
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

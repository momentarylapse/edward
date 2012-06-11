/*
 * ModeWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeWorld.h"
#include "../../Data/World/DataWorld.h"

ModeWorld *mode_world = NULL;


#define RotationMouseSpeed			0.002f
#define TerrainHeightMapFactorDec	4
#define TerrainTextureScaleDec		5
#define TerrainSizeDec				3
#define TerrainPatternDec			4
#define OSelectionAlpha				0.25f
#define OMouseOverAlpha				0.25f
#define TSelectionAlpha				0.20f
#define TMouseOverAlpha				0.20f

ModeWorld::ModeWorld()
{
	name = "World";
	parent = NULL;

	menu = HuiCreateResourceMenu("menu_world");
	data = new DataWorld;
	multi_view = ed->multi_view_3d;
	Subscribe(data);

	WorldDialog = NULL;

	ShowTerrains = false;//true;
	ShowObjects = true;
	TerrainShowTextureLevel = -1;
	ViewStage = 0;
}

ModeWorld::~ModeWorld()
{
}

bool ModeWorld::SaveAs()
{
	return false;
}



void ModeWorld::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();

	if (id == "selection_properties")
		ExecuteWorldPropertiesDialog();

	if (id == "opt_view")
		OptimizeView();
}

#define MODEL_MAX_VERTICES	65536
vector tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];

bool IsMouseOverObject(int index, void *user_data, int win, vector &tp)
{
	CModel *m = mode_world->data->Object[index].object;
	if (!m)
		return false;
	int d = m->_detail_;
	vector mv = vector(float(ed->multi_view_3d->mx), float(ed->multi_view_3d->my), 0);
	if ((d<0)||(d>2))
		return false;
	for (int i=0;i<m->skin[d]->vertex.num;i++){
		VecTransform(tmv[i], m->_matrix, m->skin[d]->vertex[i]);
		pmv[i] = mode_world->multi_view->VecProject(tmv[i],win);
	}
	float z_min=1;
	for (int mm=0;mm<m->material.num;mm++)
	for (int i=0;i<m->skin[d]->sub[mm].num_triangles;i++){
		vector a=pmv[m->skin[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[m->skin[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[m->skin[d]->sub[mm].triangle_index[i*3+2]];
		if ((a.z<=0)||(b.z<=0)||(c.z<=0)||(a.z>=1)||(b.z>=1)||(c.z>=1))
			continue;
		float f,g;
		float az=a.z,bz=b.z,cz=c.z;
		a.z=b.z=c.z=0;
		GetBaryCentric(mv,a,b,c,f,g);
		if ((f>=0)&&(g>=0)&&(f+g<=1)){
			float z=az + f*(bz-az) + g*(cz-az);
			if (z<z_min){
				z_min=z;
				tp=tmv[m->skin[d]->sub[mm].triangle_index[i*3  ]]
					+ f*(tmv[m->skin[d]->sub[mm].triangle_index[i*3+1]]-tmv[m->skin[d]->sub[mm].triangle_index[i*3  ]])
					+ g*(tmv[m->skin[d]->sub[mm].triangle_index[i*3+2]]-tmv[m->skin[d]->sub[mm].triangle_index[i*3  ]]);
			}
		}
	}
	return (z_min<1);
}

bool IsInRectObject(int index, void *user_data, int win, irect *r)
{
	CModel *m = mode_world->data->Object[index].object;
	if (!m)
		return false;
	int d = m->_detail_;
	if ((d<0)||(d>2))
		return false;
	vector min, max;
	for (int i=0;i<m->skin[d]->vertex.num;i++){
		VecTransform(tmv[i],m->_matrix,m->skin[d]->vertex[i]);
		pmv[i] = mode_world->multi_view->VecProject(tmv[i],win);
	}
	for (int mm=0;mm<m->material.num;mm++)
	for (int i=0;i<m->skin[d]->sub[mm].num_triangles;i++){
		vector a=pmv[m->skin[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[m->skin[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[m->skin[d]->sub[mm].triangle_index[i*3+2]];
		if ((a.z<=0)||(b.z<=0)||(c.z<=0)||(a.z>=1)||(b.z>=1)||(c.z>=1))
			continue;
		if (i==0)
			min = max = a;
		VecMin(min,a);
		VecMin(min,b);
		VecMin(min,c);
		VecMax(max,a);
		VecMax(max,b);
		VecMax(max,c);
	}
	return ((min.x>=r->x1)&&(min.y>=r->y1)&&(max.x<=r->x2)&&(max.y<=r->y2));
}

bool IsMouseOverTerrain(int index, void *user_data, int win, vector &tp)
{
	msg_db_r(format("IMOT index= %d",index).c_str(),3);
	ModeWorldTerrain *t = &mode_world->data->Terrain[index];
	vector mv = vector(float(ed->multi_view_3d->mx), float(ed->multi_view_3d->my), 0);
	float z_min=1;
	int x1,z1,x,z;
	for (x1=0;x1<(t->NumX-1)/32+1;x1++)
		for (z1=0;z1<(t->NumZ-1)/32+1;z1++){
			int lx=(x1*32>t->NumX-32)?(t->NumX%32):32;
			int lz=(z1*32>t->NumZ-32)?(t->NumZ%32):32;
			int x0=x1*32;
			int z0=z1*32;
			int e=t->Partition[x1][z1];
			if (e<0)	continue;
			for (x=x0;x<=x0+lx;x+=e)
				for (z=z0;z<=z0+lz;z+=e){
					int i=x*(t->NumZ+1)+z;
					pmv[i] = mode_world->multi_view->VecProject(t->Vertex[i].pos,win);
				}
			for (x=x0;x<x0+lx;x+=e)
				for (z=z0;z<z0+lz;z+=e)
					for (int i=0;i<2;i++){
						int _a_,_b_,_c_;
						if (i==0){
							_a_= x   *(t->NumZ+1)+z  ;
							_b_= x   *(t->NumZ+1)+z+e;
							_c_=(x+e)*(t->NumZ+1)+z+e;
						}else{
							_a_= x   *(t->NumZ+1)+z  ;
							_b_=(x+e)*(t->NumZ+1)+z+e;
							_c_=(x+e)*(t->NumZ+1)+z  ;
						}
						vector a=pmv[_a_],b=pmv[_b_],c=pmv[_c_];
						if ((a.z<=0)||(b.z<=0)||(c.z<=0)||(a.z>=1)||(b.z>=1)||(c.z>=1))	continue;
						float f,g;
						float az=a.z,bz=b.z,cz=c.z;
						a.z=b.z=c.z=0;
						GetBaryCentric(mv,a,b,c,f,g);
						if ((f>=0)&&(g>=0)&&(f+g<=1)){
							float z=az + f*(bz-az) + g*(cz-az);
							if (z<z_min){
								z_min=z;
								tp=t->Vertex[_a_].pos + f*(t->Vertex[_b_].pos-t->Vertex[_a_].pos) + g*(t->Vertex[_c_].pos-t->Vertex[_a_].pos);
							}
						}
				}
		}
	msg_db_l(3);
	return (z_min<1);
}

bool IsInRectTerrain(int index, void *user_data, int win, irect *r)
{
	ModeWorldTerrain *t = &mode_world->data->Terrain[index];
	vector min,max;
	for (int i=0;i<8;i++){
		vector v=t->pos+vector((i%2)==0?t->Min.x:t->Max.x,((i/2)%2)==0?t->Min.y:t->Max.y,((i/4)%2)==0?t->Min.z:t->Max.z);
		vector p = mode_world->multi_view->VecProject(v,win);
		if (i==0)
			min=max=p;
		VecMin(min,p);
		VecMax(max,p);
	}
	return ((min.x>=r->x1)&&(min.y>=r->y1)&&(max.x<=r->x2)&&(max.y<=r->y2));
}



void ModeWorld::OnLeftButtonDown()
{
}



bool ModeWorld::Save()
{
	return false;
}



void ModeWorld::OnMiddleButtonUp()
{
}



void ModeWorld::OnMouseMove()
{
}



void ModeWorld::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		foreach(data->Object, o){
			o.object->pos = o.pos;
			o.object->ang = o.Ang;
			o.object->UpdateMatrix();
		}

		multi_view->ResetData(data);

		// left -> translate
		multi_view->SetMouseAction(0, "ActionWorldMoveObjects", MultiView::ActionMove);
		// middle/right -> rotate
		multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
		multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->SetData(	MVDWorldObject,
				data->Object,
				NULL,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&IsMouseOverObject, &IsInRectObject);
	}else if (o->GetName() == "MultiView"){
		// selection
	}
}



void ModeWorld::OnKeyDown()
{
}



void ModeWorld::OnMiddleButtonDown()
{
}



void ModeWorld::OnRightButtonUp()
{
}



void ModeWorld::New()
{
	if (!ed->AllowTermination())
		return;

	data->Reset();
	multi_view->Reset();
	ed->SetMode(mode_world);
}



void ModeWorld::OnLeftButtonUp()
{
}



void ModeWorld::Draw()
{
}



void ModeWorld::OnKeyUp()
{
}



void ModeWorld::End()
{
}



void DrawSelectionObject(CModel *o, float alpha, const color &c)
{
	if (!o)
		return;
	int d = o->_detail_;
	if ((d<0)||(d>3))
		return;
	for (int i=0;i<o->material.num;i++){
		int t = o->material[i].texture[0];
		o->material[i].texture[0] = -1;
		NixSetAlpha(AlphaMaterial);
		NixSetMaterial(Black, color(alpha, 0, 0, 0), Black, 0, c);
		o->JustDraw(i, d);
		o->material[i].texture[0] = t;
	}
}


void ModeWorld::DrawWin(int win, irect dest)
{
	msg_db_r("World::DrawWin",2);
	NixEnableFog(false);
	if (false){//MVFXEnabled){
		NixSetZ(false,false);
		if (multi_view->view[win].type == ViewPerspective)
			NixDraw2D(-1,data->meta_data.BackGroundColor,r01,NixTargetRect,0);
		NixSetZ(true,true);
		NixSetFog(data->meta_data.FogMode,data->meta_data.FogStart,data->meta_data.FogEnd,data->meta_data.FogDensity,data->meta_data.FogColor);
		NixEnableFog(data->meta_data.FogEnabled);
		NixSetLightDirectional(multi_view->light,VecAng2Dir(data->meta_data.SunAng),data->meta_data.SunAmbient,data->meta_data.SunDiffuse, data->meta_data.SunSpecular);
		NixEnableLight(multi_view->light, data->meta_data.SunEnabled);
		NixSetAmbientLight(data->meta_data.Ambient);
	}
	NixSetWire(multi_view->wire_mode);
// terrain
	if (ShowTerrains)
		foreachi(data->Terrain, t, i){
			/*if (t.ViewStage < ViewStage)
				continue;*/
			NixSetWire(multi_view->wire_mode);
			NixEnableLighting(multi_view->light_enabled);
			NixSetMaterial(White,White,Black,0,Black);
			if (TerrainShowTextureLevel<0){
				NixSetShader(t.material->shader);
				NixDraw3DM(t.Texture, t.VertexBuffer, m_id);
				NixSetShader(-1);
			}else{
				int tex = -1;
				if (TerrainShowTextureLevel < t.NumTextures)
					tex = t.Texture[TerrainShowTextureLevel];
				NixDraw3D(tex, t.VertexBufferSingle, m_id);
			}

			NixSetWire(false);
			NixEnableLighting(true);
			if (t.is_selected){
				NixSetAlpha(AlphaMaterial);
				NixSetMaterial(Black,color(TSelectionAlpha,0,0,0),Black,0,Red);
				NixDraw3D(-1,t.VertexBufferSingle,m_id);
			}
			if ((multi_view->MouseOverType==MVDWorldTerrain)&&(multi_view->MouseOver==i)){
				NixSetAlpha(AlphaMaterial);
				NixSetMaterial(Black,color(TMouseOverAlpha,0,0,0),Black,0,White);
				NixDraw3D(-1,t.VertexBufferSingle,m_id);
			}
			NixSetAlpha(AlphaNone);
		}
	NixSetWire(multi_view->wire_mode);
	NixEnableLighting(multi_view->light_enabled);

msg_db_m("c",2);
// objects (models)
	if (ShowObjects){
		//GodDraw();
		//MetaDrawSorted();
		//NixSetWire(false);
		NixEnableLighting(true);

		foreach(data->Object, o){
			if (o.view_stage < ViewStage)
				continue;
			if (o.object){
				o.object->Draw(0, false, false);
				o.object->_detail_ = 0;
			}
		}
		NixSetWire(false);

		// object selection
		foreachi(data->Object, o, i)
			if (o.is_selected)
				DrawSelectionObject(o.object, OSelectionAlpha, Red);
		if ((multi_view->MouseOver>=0)&&(multi_view->MouseOverType==MVDWorldObject))
			DrawSelectionObject(data->Object[multi_view->MouseOver].object, OSelectionAlpha, White);
		NixSetAlpha(AlphaNone);
	}

// camera flight?
	/*NixSetZ(false,false);
	vector p_old,v_old=v0;
	for (int i=0;i<CamPoint.num;i++){
		vector pp=CModeAll::VecProject(CamPoint[i].Pos,win);
		if ((pp.z>0)&&(pp.z<1)){
			// path
			if (CamPoint[i].Type==CPKCamFlight)
				DrawPara3(p_old,v_old,CamPoint[i].Pos,CamPoint[i].Vel,CamPoint[i].Duration,White);
			// velocity
			if (CamPoint[i].Type==CPKCamFlight)
				NixDrawLine3D(CamPoint[i].Pos,CamPoint[i].Pos+CamPoint[i].Vel,color(1,1,1,0.5f));
			// angle
			if ((CamPoint[i].Type==CPKCamFlight)||(CamPoint[i].Type==CPKSetCamPosAng)){
				matrix m;
				MatrixRotation(m,CamPoint[i].Ang);
				vector e[3];
				VecTransform(e[0],m,vector(1,0,0));
				VecTransform(e[1],m,vector(0,1,0));
				VecTransform(e[2],m,vector(0,0,1));
				NixDrawLine3D(CamPoint[i].Pos,CamPoint[i].Pos+e[0]/Zoom3D*30,color(1,0,0.5f,0));
				NixDrawLine3D(CamPoint[i].Pos,CamPoint[i].Pos+e[1]/Zoom3D*30,color(1,0,0.5f,0));
				NixDrawLine3D(CamPoint[i].Pos,CamPoint[i].Pos+e[2]/Zoom3D*30,color(1,0,1.0f,0));
			}
			DrawInt((int)pp.x+5,(int)pp.y,i);
		}
		p_old=CamPoint[i].Pos;
		v_old=CamPoint[i].Vel;
		if (CamPoint[i].Type!=CPKCamFlight)
			v_old=v0;
	}*/
	NixSetZ(true,true);
	msg_db_l(2);
}



void ModeWorld::Start()
{
	string dir = HuiAppDirectoryStatic + SysFileName("Data/icons/toolbar/");
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Push"),_("ViewStage Push"),dir + "view_push.png","view_push");
	ed->ToolbarAddItem(_("Pop"),_("ViewStage Pop"),dir + "view_pop.png","view_pop");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Eigenschaften"),_("Eigenschaften"), dir + "configure.png", "selection_properties");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);

	OnUpdate(data);
}



void ModeWorld::OnRightButtonDown()
{
}



void ModeWorld::OnUpdateMenu()
{
	ed->Enable("undo", data->action_manager->Undoable());
	ed->Enable("redo", data->action_manager->Redoable());
}



bool ModeWorld::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDWorld, false, false))
		return false;
	if (!data->Load(ed->DialogFileComplete))
		return false;

	multi_view->Reset();
	ed->SetMode(mode_world);
	OptimizeView();
	return true;
}

void ModeWorld::ExecuteWorldPropertiesDialog()
{
	if (WorldDialog)
		return;

	WorldDialog = new WorldPropertiesDialog(ed, true, data);
	WorldDialog->Update();
	//HuiWaitTillWindowClosed(WorldDialog);
}



void ModeWorld::ExecutePropertiesSelectionDialog()
{
}



void ModeWorld::ExecuteObjectPropertiesDialog()
{
}



void ModeWorld::ExecuteTerrainPropertiesDialog()
{
}


void ModeWorld::OptimizeView()
{
	multi_view->ResetView();
	vector min, max;
	data->GetBoundaryBox(min, max);
	multi_view->pos = (max + min) / 2;
	multi_view->radius = VecLengthFuzzy(max - min) * 1.3f;

	ViewStage = 0;
	//MVFXEnabled=false;
	TerrainShowTextureLevel=-1;
	//TerrainsSelectable=false;
}





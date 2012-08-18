/*
 * ModeWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeWorld.h"
#include "../../Data/World/DataWorld.h"
#include "../../lib/x/x.h"
#include "Dialog/SelectionPropertiesDialog.h"
#include "Dialog/ObjectPropertiesDialog.h"
#include "Dialog/TerrainPropertiesDialog.h"
#include "Dialog/TerrainHeightmapDialog.h"
#include "Creation/ModeWorldCreateObject.h"
#include "Creation/ModeWorldCreateTerrain.h"
#include "../../Action/World/ActionWorldEditData.h"
#include "../../Action/World/ActionWorldSetEgo.h"

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

ModeWorld::ModeWorld() :
	Mode("World", NULL, new DataWorld, ed->multi_view_3d, "menu_world")
{
	data = dynamic_cast<DataWorld*>(data_generic);
	Subscribe(data);

	WorldDialog = NULL;

	ShowTerrains = true;
	ShowObjects = true;
	ShowEffects = false;
	TerrainShowTextureLevel = -1;
	ViewStage = 0;
}

ModeWorld::~ModeWorld()
{
}

bool ModeWorld::SaveAs()
{
	if (ed->FileDialog(FDWorld, true, false))
		return data->Save(ed->DialogFileComplete);
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

	if (id == "import_world_properties")
		ImportWorldProperties();

	if (id == "create_objects")
		ed->SetMode(new ModeWorldCreateObject(ed->cur_mode));
	if (id == "terrain_create")
		ed->SetMode(new ModeWorldCreateTerrain(ed->cur_mode));
	if (id == "terrain_load")
		LoadTerrain();

	if (id == "own_figure")
		SetEgo();
	if (id == "terrain_heightmap")
		ApplyHeightmap();

	if (id == "selection_properties")
		ExecutePropertiesDialog();

	if (id == "opt_view")
		OptimizeView();
	if (id == "show_objects")
		ToggleShowObjects();
	if (id == "show_terrains")
		ToggleShowTerrains();
	if (id == "show_fx")
		ToggleShowEffects();
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
	//msg_db_r(format("IMOT index= %d",index).c_str(),3);
	CTerrain *t = mode_world->data->Terrain[index].terrain;
	if (!t)
		return false;
#if 0
	MultiView *mv = ed->multi_view_3d;
	float mx = mv->mx;
	float my = mv->my;
	vector a = mv->VecUnProject(vector(mx, my, 0) ,win);
	vector b = mv->VecUnProject2(vector(mx, my, 0), mv->pos + mv->GetDirection(win) * mv->radius * 100, win);
	vector dir = v0;
	return t->Trace(a, b, dir, 1000000000000, tp, false);
#else
	vector mv = vector(float(ed->multi_view_3d->mx), float(ed->multi_view_3d->my), 0);
	float z_min=1;
	int x1,z1,x,z;
	for (x1=0;x1<(t->num_x-1)/32+1;x1++)
		for (z1=0;z1<(t->num_z-1)/32+1;z1++){
			int lx=(x1*32>t->num_x-32)?(t->num_x%32):32;
			int lz=(z1*32>t->num_z-32)?(t->num_z%32):32;
			int x0=x1*32;
			int z0=z1*32;
			int e=t->partition[x1][z1];
			if (e<0)	continue;
			for (int dx=0;dx<=lx;dx+=e)
				for (int dz=0;dz<=lz;dz+=e){
					int di=dx*(32+1)+dz;
					int i=(dx + x0)*(t->num_z+1)+(dz + z0);
					pmv[di] = mode_world->multi_view->VecProject(t->vertex[i],win);
				}
			for (int dx=0;dx<lx;dx+=e)
				for (int dz=0;dz<lz;dz+=e)
					for (int i=0;i<2;i++){
						int _a_,_b_,_c_;
						if (i==0){
							_a_= dx   *(32+1)+dz  ;
							_b_= dx   *(32+1)+dz+e;
							_c_=(dx+e)*(32+1)+dz+e;
						}else{
							_a_= dx   *(32+1)+dz  ;
							_b_=(dx+e)*(32+1)+dz+e;
							_c_=(dx+e)*(32+1)+dz  ;
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
								tp=t->vertex[_a_] + f*(t->vertex[_b_]-t->vertex[_a_]) + g*(t->vertex[_c_]-t->vertex[_a_]);
							}
						}
				}
		}
	//msg_db_l(3);
	return (z_min<1);
#endif
}

bool IsInRectTerrain(int index, void *user_data, int win, irect *r)
{
	CTerrain *t = mode_world->data->Terrain[index].terrain;
	vector min,max;
	for (int i=0;i<8;i++){
		vector v=t->pos+vector((i%2)==0?t->min.x:t->max.x,((i/2)%2)==0?t->min.y:t->max.y,((i/4)%2)==0?t->min.z:t->max.z);
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
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
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
		data->UpdateData();

		multi_view->ResetData(data);

		// left -> translate
		multi_view->SetMouseAction(0, "ActionWorldMoveSelection", MultiView::ActionMove);
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
		multi_view->SetData(	MVDWorldTerrain,
				data->Terrain,
				NULL,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&IsMouseOverTerrain, &IsInRectTerrain);
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
	OptimizeView();
	ed->SetMode(mode_world);
}



void ModeWorld::OnLeftButtonUp()
{
}



void ModeWorld::OnDraw()
{
	cur_cam->pos = multi_view->pos;

	int num_ob = data->GetSelectedObjects();
	int num_te = data->GetSelectedTerrains();
	if (num_ob + num_te > 0){
		ed->DrawStr(10, 100, format("obj: %d", num_ob));
		ed->DrawStr(10, 120, format("ter: %d", num_te));
	}
}



void ModeWorld::OnKeyUp()
{
}



void ModeWorld::OnEnd()
{
	if (WorldDialog)
		delete(WorldDialog);
	WorldDialog = NULL;

	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
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

void DrawTerrainColored(CTerrain *t, const color &c, float alpha)
{
	NixSetWire(false);
	NixEnableLighting(true);
	NixSetAlpha(AlphaMaterial);

	// save terrain data
	color am = t->material->ambient;
	color di = t->material->diffuse;
	color sp = t->material->specular;
	color em = t->material->emission;
	int shader = t->material->shader;
	int texture[TERRAIN_MAX_TEXTURES];
	for (int i=0;i<t->num_textures;i++)
		texture[i] = t->texture[i];

	// alter data
	t->material->ambient = Black;
	t->material->diffuse = color(alpha, 0, 0, 0);
	t->material->specular = Black;
	t->material->emission = c;
	t->material->shader = -1;
	for (int i=0;i<t->num_textures;i++)
		t->texture[i] = -1;

	t->Draw();

	// restore data
	t->material->shader = shader;
	for (int i=0;i<t->num_textures;i++)
		t->texture[i] = texture[i];
	t->material->ambient = am;
	t->material->diffuse = di;
	t->material->specular = sp;
	t->material->emission = em;

	NixSetAlpha(AlphaNone);
	NixSetWire(mode_world->multi_view->wire_mode);
	NixEnableLighting(mode_world->multi_view->light_enabled);
}

void ModeWorld::OnDrawWin(int win, irect dest)
{
	msg_db_r("World::DrawWin",2);

	if (ShowEffects){
		if (multi_view->view[win].type == ViewPerspective)
			data->meta_data.DrawBackground();
		data->meta_data.ApplyToDraw();
	}

// terrain
	if (ShowTerrains)
		foreachi(data->Terrain, t, i){
			if (!t.terrain)
				continue;
			/*if (t.ViewStage < ViewStage)
				continue;*/

			/*if (TerrainShowTextureLevel<0){
				NixSetShader(t.material->shader);
				NixDraw3DM(t.Texture, t.VertexBuffer, m_id);
				NixSetShader(-1);
			}else{
				int tex = -1;
				if (TerrainShowTextureLevel < t.NumTextures)
					tex = t.Texture[TerrainShowTextureLevel];
				NixDraw3D(tex, t.VertexBufferSingle, m_id);
			}*/
			t.terrain->Draw();

			if (t.is_selected)
				DrawTerrainColored(t.terrain, Red, TSelectionAlpha);
			if ((multi_view->MouseOverType==MVDWorldTerrain)&&(multi_view->MouseOver==i))
				DrawTerrainColored(t.terrain, White, TMouseOverAlpha);
		}
	NixSetWire(multi_view->wire_mode);
	NixEnableLighting(multi_view->light_enabled);

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
			else if (o.is_special)
				DrawSelectionObject(o.object, OSelectionAlpha, Green);
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
	NixEnableFog(false);

	msg_db_l(2);
}



void ModeWorld::OnStart()
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

	ed->Check("show_objects", ShowObjects);
	ed->Check("show_terrains", ShowTerrains);
	ed->Check("show_fx", ShowEffects);
}



bool ModeWorld::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDWorld, false, false))
		return false;
	ed->progress->Start(_("Lade Welt"), 0);
	bool ok = data->Load(ed->DialogFileComplete);
	ed->progress->End();
	if (!ok)
		return false;

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



void ModeWorld::ExecutePropertiesDialog()
{
	int num_o = data->GetSelectedObjects();
	int num_t = data->GetSelectedTerrains();

	if (num_o + num_t == 0){
		// nothing selected -> world
		ExecuteWorldPropertiesDialog();
	}else if ((num_o == 1) && (num_t == 0)){
		// single object -> object
		foreachi(data->Object, o, i)
			if (o.is_selected)
				ExecuteObjectPropertiesDialog(i);
	}else if ((num_o == 0) && (num_t == 1)){
		// single terrain -> terrain
		foreachi(data->Terrain, t, i)
			if (t.is_selected)
				ExecuteTerrainPropertiesDialog(i);
	}else{
		// multiple selections -> choose
		ExecuteSelectionPropertiesDialog();
	}
}



void ModeWorld::ExecuteSelectionPropertiesDialog()
{
	//ExecuteWorldPropertiesDialog();
	int sel_type, sel_index;

	SelectionPropertiesDialog *dlg = new SelectionPropertiesDialog(ed, false, data, &sel_type, &sel_index);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);

	if (sel_type >= 0){
		if (sel_type == FDWorld){
			ExecuteWorldPropertiesDialog();
		}else if (sel_type == FDModel){
			ExecuteObjectPropertiesDialog(sel_index);
		}else if (sel_type==FDTerrain){
			ExecuteTerrainPropertiesDialog(sel_index);
		}/*if (sel_type == FDCameraFlight){
			CamPointDialogIndex=PropertySelectionIndex[PropertySelectionChosen];
			ExecuteCamPointDialog();
		}*/
	}
}



void ModeWorld::ExecuteObjectPropertiesDialog(int index)
{
	ObjectPropertiesDialog *dlg = new ObjectPropertiesDialog(ed, false, data, index);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);
}



void ModeWorld::ExecuteTerrainPropertiesDialog(int index)
{
	TerrainPropertiesDialog *dlg = new TerrainPropertiesDialog(ed, false, data, index);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);
}


void ModeWorld::OptimizeView()
{
	multi_view->ResetView();
	vector min, max;
	data->GetBoundaryBox(min, max);
	multi_view->pos = (max + min) / 2;
	if (VecLengthFuzzy(max - min) > 0)
		multi_view->radius = VecLengthFuzzy(max - min) * 1.3f;

	ViewStage = 0;
	//ShowEffects = false;
	TerrainShowTextureLevel = -1;
	//TerrainsSelectable=false;
}

void ModeWorld::LoadTerrain()
{
	if (ed->FileDialog(FDTerrain, false, true))
		data->AddTerrain(ed->DialogFileNoEnding, multi_view->pos);
}

void ModeWorld::SetEgo()
{
	if (data->GetSelectedObjects() != 1){
		ed->SetMessage(_("Es muss genau ein Objekt markiert sein!"));
		return;
	}
	foreachi(data->Object, o, i)
		if (o.is_selected)
			data->Execute(new ActionWorldSetEgo(i));
}

void ModeWorld::ToggleShowEffects()
{
	ShowEffects = !ShowEffects;
	ed->UpdateMenu();
	ed->ForceRedraw();
}



void ModeWorld::ToggleShowObjects()
{
	ShowObjects = !ShowObjects;
	ed->UpdateMenu();
	ed->ForceRedraw();
}



void ModeWorld::ToggleShowTerrains()
{
	ShowTerrains = !ShowTerrains;
	ed->UpdateMenu();
	ed->ForceRedraw();
}


void ModeWorld::ImportWorldProperties()
{
	if (ed->FileDialog(FDWorld, false, false)){
		DataWorld w;
		if (w.Load(ed->DialogFileComplete, false))
			data->Execute(new ActionWorldEditData(w.meta_data));
		else
			ed->ErrorBox(_("Angegebene Welt konnte nicht korrekt geladen werden!"));
	}
}

void ModeWorld::ApplyHeightmap()
{
	if (data->GetSelectedTerrains() == 0){
		ed->SetMessage(_("Es muss mindestens ein Terrain markiert sein!"));
		return;
	}
	TerrainHeightmapDialog *dlg = new TerrainHeightmapDialog(ed, false, data);
	dlg->Update();

	HuiWaitTillWindowClosed(dlg);
}







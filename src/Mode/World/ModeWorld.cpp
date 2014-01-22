/*
 * ModeWorld.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "../../MultiView/MultiView.h"
#include "../../MultiView/Window.h"
#include "ModeWorld.h"
#include "../../Data/World/DataWorld.h"
#include "../../Data/World/DataCamera.h"
#include "../../lib/nix/nix.h"
#include "../../x/camera.h"
#include "../../x/world.h"
#include "../../x/material.h"
#include "../../x/model.h"
#include "../../x/object.h"
#include "../../x/terrain.h"
#include "Dialog/SelectionPropertiesDialog.h"
#include "Dialog/ObjectPropertiesDialog.h"
#include "Dialog/TerrainPropertiesDialog.h"
#include "Dialog/TerrainHeightmapDialog.h"
#include "Dialog/LightmapDialog.h"
#include "Creation/ModeWorldCreateObject.h"
#include "Creation/ModeWorldCreateTerrain.h"
#include "Camera/ModeWorldCamera.h"
#include "Terrain/ModeWorldEditTerrain.h"
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
	Mode<DataWorld>("World", NULL, new DataWorld, ed->multi_view_3d, "menu_world")
{
	Subscribe(data);

	WorldDialog = NULL;

	ShowTerrains = true;
	ShowObjects = true;
	ShowEffects = false;
	TerrainShowTextureLevel = -1;

	mode_world_camera = new ModeWorldCamera(this, new DataCamera);
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


	if (id == "copy")
		Copy();
	if (id == "paste")
		Paste();
	if (id == "delete")
		data->DeleteSelection();

	if (id == "import_world_properties")
		ImportWorldProperties();

	if (id == "create_objects")
		ed->SetMode(new ModeWorldCreateObject(ed->cur_mode));
	if (id == "terrain_create")
		ed->SetMode(new ModeWorldCreateTerrain(ed->cur_mode));
	if (id == "terrain_load")
		LoadTerrain();

	if (id == "camscript_create")
		ed->SetMode(mode_world_camera);
	if (id == "camscript_load")
		if (ed->FileDialog(FDCameraFlight, false, true)){
			if (mode_world_camera->data->Load(ed->DialogFileComplete))
				ed->SetMode(mode_world_camera);
			else
				mode_world_camera->data->Reset();
		}
	if (id == "edit_terrain_vertices")
		ed->SetMode(new ModeWorldEditTerrain(ed->cur_mode));
	if (id == "create_lightmap")
		ExecuteLightmapDialog();

	if (id == "own_figure")
		SetEgo();
	if (id == "terrain_heightmap")
		ApplyHeightmap();

	if (id == "selection_properties")
		ExecutePropertiesDialog();

	if (id == "show_objects")
		ToggleShowObjects();
	if (id == "show_terrains")
		ToggleShowTerrains();
	if (id == "show_fx")
		ToggleShowEffects();

	if (id == "select")
		SetMouseAction(MultiView::ActionSelect);
	if (id == "translate")
		SetMouseAction(MultiView::ActionMove);
	if (id == "rotate")
		SetMouseAction(MultiView::ActionRotate);
}

#define MODEL_MAX_VERTICES	65536
vector tmv[MODEL_MAX_VERTICES*5],pmv[MODEL_MAX_VERTICES*5];
bool tvm[MODEL_MAX_VERTICES*5];

bool WorldObject::Hover(MultiView::Window *win, vector &mv, vector &tp, float &z, void *user_data)
{
	Object *o = object;
	if (!o)
		return false;
	int d = o->_detail_;
	if ((d<0)||(d>2))
		return false;
	for (int i=0;i<o->skin[d]->vertex.num;i++){
		tmv[i] = o->_matrix * o->skin[d]->vertex[i];
		pmv[i] = win->Project(tmv[i]);
	}
	float z_min=1;
	for (int mm=0;mm<o->material.num;mm++)
	for (int i=0;i<o->skin[d]->sub[mm].num_triangles;i++){
		vector a=pmv[o->skin[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[o->skin[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[o->skin[d]->sub[mm].triangle_index[i*3+2]];
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
				tp=tmv[o->skin[d]->sub[mm].triangle_index[i*3  ]]
					+ f*(tmv[o->skin[d]->sub[mm].triangle_index[i*3+1]]-tmv[o->skin[d]->sub[mm].triangle_index[i*3  ]])
					+ g*(tmv[o->skin[d]->sub[mm].triangle_index[i*3+2]]-tmv[o->skin[d]->sub[mm].triangle_index[i*3  ]]);
			}
		}
	}
	z = z_min;
	return (z_min<1);
}

bool WorldObject::InRect(MultiView::Window *win, rect &r, void *user_data)
{
	Object *m = object;
	if (!m)
		return false;
	int d = m->_detail_;
	if ((d<0)||(d>2))
		return false;
	vector min, max;
	for (int i=0;i<m->skin[d]->vertex.num;i++){
		tmv[i] = m->_matrix * m->skin[d]->vertex[i];
		pmv[i] = win->Project(tmv[i]);
		if (r.inside(pmv[i].x, pmv[i].y))
			return true;
	}
	return false;
	for (int mm=0;mm<m->material.num;mm++)
	for (int i=0;i<m->skin[d]->sub[mm].num_triangles;i++){
		vector a=pmv[m->skin[d]->sub[mm].triangle_index[i*3  ]];
		vector b=pmv[m->skin[d]->sub[mm].triangle_index[i*3+1]];
		vector c=pmv[m->skin[d]->sub[mm].triangle_index[i*3+2]];
		if ((a.z<=0)||(b.z<=0)||(c.z<=0)||(a.z>=1)||(b.z>=1)||(c.z>=1))
			continue;
		if (i==0)
			min = max = a;
		min._min(a);
		min._min(b);
		min._min(c);
		max._max(a);
		max._max(b);
		max._max(c);
	}
	return ((min.x>=r.x1)&&(min.y>=r.y1)&&(max.x<=r.x2)&&(max.y<=r.y2));
}

bool WorldTerrain::Hover(MultiView::Window *win, vector &mv, vector &tp, float &z, void *user_data)
{
	//msg_db_f(format("IMOT index= %d",index).c_str(),3);
	Terrain *t = terrain;
	if (!t)
		return false;
	float r = win->cam->radius * 100;
	vector a = win->Unproject(mv);
	vector b = win->Unproject(mv, win->cam->pos + win->GetDirection() * r);
	TraceData td;
	bool hit = t->Trace(a, b, v_0, r, td, false);
	tp = td.point;
	z = win->Project(tp).z;
	return hit;
}

bool WorldTerrain::InRect(MultiView::Window *win, rect &r, void *user_data)
{
	Terrain *t = terrain;
	vector min,max;
	for (int i=0;i<8;i++){
		vector v=t->pos+vector((i%2)==0?t->min.x:t->max.x,((i/2)%2)==0?t->min.y:t->max.y,((i/4)%2)==0?t->min.z:t->max.z);
		vector p = win->Project(v);
		if (i==0)
			min=max=p;
		min._min(p);
		max._max(p);
	}
	return ((min.x>=r.x1)&&(min.y>=r.y1)&&(max.x<=r.x2)&&(max.y<=r.y2));
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

		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->SetData(	MVDWorldObject,
				data->Objects,
				NULL,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
		multi_view->SetData(	MVDWorldTerrain,
				data->Terrains,
				NULL,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
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
	cur_cam->pos = multi_view->cam.pos;

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

	ed->toolbar[HuiToolbarTop]->Reset();
	ed->toolbar[HuiToolbarTop]->Enable(false);
}



void DrawSelectionObject(Model *o, float alpha, const color &c)
{
	if (!o)
		return;
	int d = o->_detail_;
	if ((d<0)||(d>3))
		return;
	for (int i=0;i<o->material.num;i++){
		NixTexture *t[MATERIAL_MAX_TEXTURES];
		for (int j=0;j<o->material[i].num_textures;j++)
			t[j] = NULL;
		NixSetTextures(t, o->material[i].num_textures);
		NixSetAlpha(AlphaMaterial);
		NixSetMaterial(Black, color(alpha, 0, 0, 0), Black, 0, c);
		o->JustDraw(i, d);
	}
}

void DrawTerrainColored(Terrain *t, const color &c, float alpha)
{
	NixSetWire(false);
	NixEnableLighting(true);
	NixSetAlpha(AlphaMaterial);

	// save terrain data
	Material *temp = t->material;

	// alter data
	Material *m = &mode_world->temp_material;
	m->ambient = Black;
	m->diffuse = color(alpha, 0, 0, 0);
	m->specular = Black;
	m->emission = c;
	m->shader = NULL;
	m->num_textures = t->material->num_textures;
	for (int i=0;i<t->material->num_textures;i++)
		m->texture[i] = NULL;

	t->material = m;

	t->Draw();

	// restore data
	t->material = temp;

	NixSetAlpha(AlphaNone);
	NixSetWire(mode_world->multi_view->wire_mode);
	NixEnableLighting(mode_world->multi_view->light_enabled);
}

void ModeWorld::OnDrawWin(MultiView::Window *win)
{
	msg_db_f("World::DrawWin",2);

	if (ShowEffects){
		if (win->type == MultiView::ViewPerspective)
			data->meta_data.DrawBackground();
		data->meta_data.ApplyToDraw();
	}

// terrain
	if (ShowTerrains){
		foreachi(WorldTerrain &t, data->Terrains, i){
			if (!t.terrain)
				continue;
			if (t.view_stage < multi_view->view_stage)
				continue;

			t.terrain->Draw();

			if (t.is_selected)
				DrawTerrainColored(t.terrain, Red, TSelectionAlpha);
			if ((multi_view->hover.type == MVDWorldTerrain) && (multi_view->hover.index == i))
				DrawTerrainColored(t.terrain, White, TMouseOverAlpha);
		}
	}
	NixSetWire(multi_view->wire_mode);
	NixEnableLighting(multi_view->light_enabled);

// objects (models)
	if (ShowObjects){
		//GodDraw();
		//MetaDrawSorted();
		//NixSetWire(false);

		foreach(WorldObject &o, data->Objects){
			if (o.view_stage < multi_view->view_stage)
				continue;
			if (o.object){
				for (int i=0;i<o.object->material.num;i++)
					o.object->material[i].shader = NULL;
				o.object->Draw(0, false, false);
				o.object->_detail_ = 0;
			}
		}
		NixSetWire(false);
		NixEnableLighting(true);

		// object selection
		foreachi(WorldObject &o, data->Objects, i)
			if (o.is_selected)
				DrawSelectionObject(o.object, OSelectionAlpha, Red);
			else if (o.is_special)
				DrawSelectionObject(o.object, OSelectionAlpha, Green);
		if ((multi_view->hover.index >= 0) && (multi_view->hover.type == MVDWorldObject))
			DrawSelectionObject(data->Objects[multi_view->hover.index].object, OSelectionAlpha, White);
		NixSetAlpha(AlphaNone);
		NixEnableLighting(multi_view->light_enabled);
	}


	NixSetZ(true,true);
	NixEnableFog(false);
}



void ModeWorld::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->AddItem(L("new"),dir + "new.png","new");
	t->AddItem(L("open"),dir + "open.png","open");
	t->AddItem(L("save"),dir + "save.png","save");
	t->AddSeparator();
	t->AddItem(L("undo"),dir + "undo.png","undo");
	t->AddItem(L("redo"),dir + "redo.png","redo");
	t->AddSeparator();
	t->AddItem(_("Push"),dir + "view_push.png","view_push");
	t->AddItem(_("Pop"),dir + "view_pop.png","view_pop");
	t->AddSeparator();
	t->AddItem(_("Eigenschaften"), dir + "configure.png", "selection_properties");
	t->AddSeparator();
	t->AddItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->AddItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->AddItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->Enable(true);
	t->Configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->Enable(false);

	SetMouseAction(MultiView::ActionSelect);

	OnUpdate(data);
}

void ModeWorld::SetMouseAction(int mode)
{
	mouse_action = mode;
	if (mode == MultiView::ActionMove)
		multi_view->SetMouseAction("ActionWorldMoveSelection", mode);
	else if (mode == MultiView::ActionRotate)
		multi_view->SetMouseAction("ActionWorldRotateObjects", mode);
	else
		multi_view->SetMouseAction("", mode);
	ed->UpdateMenu();
}



void ModeWorld::OnRightButtonDown()
{
}



void ModeWorld::OnUpdateMenu()
{
	ed->Enable("undo", data->action_manager->Undoable());
	ed->Enable("redo", data->action_manager->Redoable());

	ed->Enable("copy", Copyable());
	ed->Enable("paste", Pasteable());

	ed->Check("show_objects", ShowObjects);
	ed->Check("show_terrains", ShowTerrains);
	ed->Check("show_fx", ShowEffects);

	ed->Check("select", mouse_action == MultiView::ActionSelect);
	ed->Check("translate", mouse_action == MultiView::ActionMove);
	ed->Check("rotate", mouse_action == MultiView::ActionRotate);
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
	WorldDialog->Show();
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
		foreachi(WorldObject &o, data->Objects, i)
			if (o.is_selected)
				ExecuteObjectPropertiesDialog(i);
	}else if ((num_o == 0) && (num_t == 1)){
		// single terrain -> terrain
		foreachi(WorldTerrain &t, data->Terrains, i)
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
	dlg->Run();

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
	dlg->Run();
}



void ModeWorld::ExecuteTerrainPropertiesDialog(int index)
{
	TerrainPropertiesDialog *dlg = new TerrainPropertiesDialog(ed, false, data, index);
	dlg->Run();
}

void ModeWorld::ExecuteLightmapDialog()
{
	LightmapDialog *dlg = new LightmapDialog(ed, false, data);
	dlg->Run();
}


bool ModeWorld::OptimizeView()
{
	multi_view->ResetView();
	vector min, max;
	data->GetBoundaryBox(min, max);
	multi_view->SetViewBox(min, max);

	//ShowEffects = false;
	TerrainShowTextureLevel = -1;
	//TerrainsSelectable=false;
	return true;
}

void ModeWorld::LoadTerrain()
{
	if (ed->FileDialog(FDTerrain, false, true))
		data->AddTerrain(ed->DialogFileNoEnding, multi_view->cam.pos);
}

void ModeWorld::SetEgo()
{
	if (data->GetSelectedObjects() != 1){
		ed->SetMessage(_("Es muss genau ein Objekt markiert sein!"));
		return;
	}
	foreachi(WorldObject &o, data->Objects, i)
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
	dlg->Run();
}







void ModeWorld::Copy()
{
	data->Copy(temp_objects, temp_terrains);

	OnUpdateMenu();
	ed->SetMessage(format(_("%d Objekte, %d Terrains kopiert"), temp_objects.num, temp_terrains.num));
}

void ModeWorld::Paste()
{
	data->Paste(temp_objects, temp_terrains);
	ed->SetMessage(format(_("%d Objekte, %d Terrains eingef&ugt"), temp_objects.num, temp_terrains.num));
}

bool ModeWorld::Copyable()
{
	return (data->GetSelectedObjects() + data->GetSelectedTerrains()) > 0;
}

bool ModeWorld::Pasteable()
{
	return (temp_objects.num + temp_terrains.num) > 0;
}



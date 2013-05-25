/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeMaterial.h"
#include "../../Data/Material/DataMaterial.h"
#include "Dialog/MaterialPropertiesDialog.h"
#include "Dialog/MaterialPhysicsDialog.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/Geometry/ModelGeometryTorus.h"
#include "../../Data/Model/Geometry/ModelGeometryTeapot.h"

const int MATERIAL_NUMX = 48;
const int MATERIAL_NUMY = 24;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial() :
	Mode("Material", NULL, new DataMaterial, ed->multi_view_3d, "menu_material")
{
	data = dynamic_cast<DataMaterial*>(data_generic);
	Subscribe(data);

	AppearanceDialog = NULL;

	MaterialVB[1] = VBTemp;
	for (int i=2;i<MATERIAL_MAX_TEXTURES;i++)
		MaterialVB[i] = NixCreateVB(MATERIAL_NUMX * MATERIAL_NUMY * 2, i);
}

ModeMaterial::~ModeMaterial()
{
}

void ModeMaterial::New()
{
	data->Reset();
	OptimizeView();
	ed->SetMode(mode_material);
}



bool ModeMaterial::Save()
{
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
}



void ModeMaterial::OnDraw()
{
}



void ModeMaterial::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		data->UpdateTextures();
		NixUnrefShader(data->Shader);
		data->Shader = data->Appearance.GetShader();
	}
}



void ModeMaterial::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	if (id == "appearance")
		ExecuteAppearanceDialog();
	if (id == "physics")
		ExecutePhysicsDialog();

	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();
}

void ModeMaterial::ExecuteAppearanceDialog()
{
	if (AppearanceDialog)
		return;

	AppearanceDialog = new MaterialPropertiesDialog(ed, true, data);

	AppearanceDialog->Update();
	//HuiWaitTillWindowClosed(AppearanceDialog);
}

void ModeMaterial::ExecutePhysicsDialog()
{
	CHuiWindow *dlg = new MaterialPhysicsDialog(ed, true, data);

	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}


void ModeMaterial::OnDrawWin(int win)
{
	data->ApplyForRendering();

	NixDraw3D(MaterialVB[max(data->Appearance.NumTextureLevels, 1)]);

	NixSetTexture(-1);
	NixSetShader(-1);

	NixSetAlpha(AlphaNone);
	NixSetZ(true,true);
}



bool ModeMaterial::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDMaterial, false, false))
		return false;
	if (!data->Load(ed->DialogFileComplete))
		return false;

	OptimizeView();
	ed->SetMode(mode_material);
	return true;
}



void ModeMaterial::OnEnd()
{
	if (AppearanceDialog)
		delete(AppearanceDialog);
	AppearanceDialog = NULL;

	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



bool ModeMaterial::SaveAs()
{
	if (ed->FileDialog(FDMaterial, true, false))
		return data->Save(ed->DialogFileComplete);
	return false;
}



void ModeMaterial::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Eigenschaften"),_("Eigenschaften"), dir + "configure.png", "appearance");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
	multi_view->MVRectable = false;

	//ModelGeometryTorus geo = ModelGeometryTorus(v_0, e_z, MATERIAL_RADIUS1, MATERIAL_RADIUS2, MATERIAL_NUMX, MATERIAL_NUMY);
	ModelGeometryTeapot geo = ModelGeometryTeapot(v_0, MATERIAL_RADIUS1, 6);
	geo.Smoothen();

	for (int i=1;i<MATERIAL_MAX_TEXTURES;i++){
		int vb = MaterialVB[i];
		NixVBClear(vb);
		geo.Preview(vb, i);
	}
}

bool ModeMaterial::OptimizeView()
{
	multi_view->ResetView();
	vector r = vector(MATERIAL_RADIUS1 + MATERIAL_RADIUS2, MATERIAL_RADIUS1 + MATERIAL_RADIUS2, MATERIAL_RADIUS2);
	multi_view->SetViewBox(-r, r);
	return true;
}




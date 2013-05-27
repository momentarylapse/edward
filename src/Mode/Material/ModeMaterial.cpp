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
#include "../../Data/Model/Geometry/ModelGeometryCube.h"
#include "../../Data/Model/Geometry/ModelGeometryBall.h"
#include "../../Data/Model/Geometry/ModelGeometryPlatonic.h"
#include "../../Data/Model/Geometry/ModelGeometryTorus.h"
#include "../../Data/Model/Geometry/ModelGeometryTorusKnot.h"
#include "../../Data/Model/Geometry/ModelGeometryTeapot.h"

const int MATERIAL_NUMX = 48;
const int MATERIAL_NUMY = 24;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial() :
	Mode("Material", NULL, new DataMaterial, ed->multi_view_3d, "menu_material")
{
	geo = NULL;
	data = dynamic_cast<DataMaterial*>(data_generic);
	Subscribe(data);

	AppearanceDialog = NULL;

	MaterialVB[1] = VBTemp;
	for (int i=2;i<MATERIAL_MAX_TEXTURES;i++)
		MaterialVB[i] = NixCreateVB(MATERIAL_NUMX * MATERIAL_NUMY * 2, i);

	shape_type = HuiConfigReadStr("MaterialShapeType", "teapot");
	shape_smooth = HuiConfigReadBool("MaterialShapeSmooth", true);
}

ModeMaterial::~ModeMaterial()
{
	if (geo)
		delete(geo);
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

	if (id == "material_shape_smooth")
		SetShapeSmooth(!shape_smooth);
	if (id == "material_shape_cube")
		SetShapeType("cube");
	if (id == "material_shape_ball")
		SetShapeType("ball");
	if (id == "material_shape_torus")
		SetShapeType("torus");
	if (id == "material_shape_torusknot")
		SetShapeType("torusknot");
	if (id == "material_shape_icosahedron")
		SetShapeType("icosahedron");
	if (id == "material_shape_teapot")
		SetShapeType("teapot");
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


void ModeMaterial::OnDrawWin(MultiViewWindow *win)
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

	UpdateShape();
}

void ModeMaterial::SetShapeType(const string &type)
{
	shape_type = type;
	HuiConfigWriteStr("MaterialShapeType", shape_type);
	UpdateShape();
}

void ModeMaterial::SetShapeSmooth(bool smooth)
{
	shape_smooth = smooth;
	HuiConfigWriteBool("MaterialShapeSmooth", shape_smooth);
	UpdateShape();
}

void ModeMaterial::UpdateShape()
{
	bool needs_opt_view = !geo;
	if (geo)
		delete(geo);
	if (shape_type == "torus")
		geo = new ModelGeometryTorus(v_0, e_z, MATERIAL_RADIUS1, MATERIAL_RADIUS2, MATERIAL_NUMX, MATERIAL_NUMY);
	else if (shape_type == "torusknot")
		geo = new ModelGeometryTorusKnot(v_0, e_z, MATERIAL_RADIUS1, 40, 22, 2, 5, 60, 16);
	else if (shape_type == "teapot")
		geo = new ModelGeometryTeapot(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "cube")
		geo = new ModelGeometryCube(-vector(1,1,1) * MATERIAL_RADIUS1/2, e_x * MATERIAL_RADIUS1, e_y * MATERIAL_RADIUS1, e_z * MATERIAL_RADIUS1, 1, 1, 1);
		//geo = new ModelGeometryPlatonic(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "icosahedron")
		geo = new ModelGeometryPlatonic(v_0, MATERIAL_RADIUS1, 20);
	else //if (shape_type == "ball")
		geo = new ModelGeometryBall(v_0, MATERIAL_RADIUS1, 32, 16);
	if (shape_smooth)
		geo->Smoothen();

	for (int i=1;i<MATERIAL_MAX_TEXTURES;i++){
		int vb = MaterialVB[i];
		NixVBClear(vb);
		geo->Preview(vb, i);
	}
	OnUpdateMenu();
	if (needs_opt_view)
		OptimizeView();
}

bool ModeMaterial::OptimizeView()
{
	multi_view->ResetView();
	if (geo){
		vector min, max;
		geo->GetBoundingBox(min, max);
		multi_view->SetViewBox(min, max);
	}
	return true;
}

void ModeMaterial::OnUpdateMenu()
{
	ed->Check("material_shape_smooth", shape_smooth);
	ed->Check("material_shape_cube", shape_type == "cube");
	ed->Check("material_shape_ball", shape_type == "ball");
	ed->Check("material_shape_torus", shape_type == "torus");
	ed->Check("material_shape_torusknot", shape_type == "torusknot");
	ed->Check("material_shape_teapot", shape_type == "teapot");
	ed->Check("material_shape_icosahedron", shape_type == "icosahedron");
}




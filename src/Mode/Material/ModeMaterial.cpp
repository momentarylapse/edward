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
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/Geometry/GeometryCube.h"
#include "../../Data/Model/Geometry/GeometryBall.h"
#include "../../Data/Model/Geometry/GeometryPlatonic.h"
#include "../../Data/Model/Geometry/GeometryTorus.h"
#include "../../Data/Model/Geometry/GeometryTorusKnot.h"
#include "../../Data/Model/Geometry/GeometryTeapot.h"

const int MATERIAL_NUMX = 48;
const int MATERIAL_NUMY = 24;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial() :
	Mode("Material", NULL, new DataMaterial, ed->multi_view_3d, "menu_material")
{
	geo = NULL;
	Subscribe(data);

	AppearanceDialog = NULL;

	MaterialVB[1] = VBTemp;
	for (int i=2;i<=MATERIAL_MAX_TEXTURES;i++)
		MaterialVB[i] = new NixVertexBuffer(i);

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
		if (data->Shader)
			data->Shader->unref();
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


void ModeMaterial::OnDrawWin(MultiViewWindow *win)
{
	data->ApplyForRendering();

	NixDraw3D(MaterialVB[max(data->Appearance.NumTextureLevels, 1)]);

	NixSetTexture(NULL);
	NixSetShader(NULL);

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
	delete(AppearanceDialog);

	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->Enable(false);
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
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->AddItem(L("new"),dir + "new.png","new");
	t->AddItem(L("open"),dir + "open.png","open");
	t->AddItem(L("save"),dir + "save.png","save");
	t->AddSeparator();
	t->AddItem(L("undo"),dir + "undo.png","undo");
	t->AddItem(L("redo"),dir + "redo.png","redo");
	t->Enable(true);
	t->Configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->Enable(false);
	multi_view->allow_rect = false;

	AppearanceDialog = new MaterialPropertiesDialog(ed, data);

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
		geo = new GeometryTorus(v_0, e_z, MATERIAL_RADIUS1, MATERIAL_RADIUS2, MATERIAL_NUMX, MATERIAL_NUMY);
	else if (shape_type == "torusknot")
		geo = new GeometryTorusKnot(v_0, e_z, MATERIAL_RADIUS1, 40, 22, 2, 5, 60, 16);
	else if (shape_type == "teapot")
		geo = new GeometryTeapot(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "cube")
		geo = new GeometryCube(-vector(1,1,1) * MATERIAL_RADIUS1/2, e_x * MATERIAL_RADIUS1, e_y * MATERIAL_RADIUS1, e_z * MATERIAL_RADIUS1, 1, 1, 1);
		//geo = new ModelGeometryPlatonic(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "icosahedron")
		geo = new GeometryPlatonic(v_0, MATERIAL_RADIUS1, 20);
	else //if (shape_type == "ball")
		geo = new GeometryBall(v_0, MATERIAL_RADIUS1, 32, 16);
	if (shape_smooth)
		geo->Smoothen();

	for (int i=1;i<=MATERIAL_MAX_TEXTURES;i++){
		NixVertexBuffer *vb = MaterialVB[i];
		vb->clear();
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




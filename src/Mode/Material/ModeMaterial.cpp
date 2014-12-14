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
#include "../../MultiView/MultiView.h"
#include "../../lib/nix/nix.h"

const int MATERIAL_NUMX = 48;
const int MATERIAL_NUMY = 24;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial() :
	Mode("Material", NULL, new DataMaterial, ed->multi_view_3d, "menu_material")
{
	geo = NULL;
	subscribe(data);

	AppearanceDialog = NULL;

	MaterialVB[1] = VBTemp;
	for (int i=2;i<=MATERIAL_MAX_TEXTURES;i++)
		MaterialVB[i] = new NixVertexBuffer(i);

	shape_type = HuiConfig.getStr("MaterialShapeType", "teapot");
	shape_smooth = HuiConfig.getBool("MaterialShapeSmooth", true);
}

ModeMaterial::~ModeMaterial()
{
	if (geo)
		delete(geo);
}

void ModeMaterial::_new()
{
	data->reset();
	optimizeView();
	ed->setMode(mode_material);
}



bool ModeMaterial::save()
{
	if (data->filename == "")
		return saveAs();
	return data->save(data->filename);
}



void ModeMaterial::onDraw()
{
}



void ModeMaterial::onUpdate(Observable *o)
{
	if (o->getName() == "Data"){
		data->UpdateTextures();
		if (data->Shader)
			data->Shader->unref();
		data->Shader = data->Appearance.GetShader();
	}
}



void ModeMaterial::onCommand(const string & id)
{
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		saveAs();

	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

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


void ModeMaterial::onDrawWin(MultiView::Window *win)
{
	data->ApplyForRendering();

	NixDraw3D(MaterialVB[max(data->Appearance.NumTextureLevels, 1)]);

	NixSetTexture(NULL);
	NixSetShader(NULL);

	NixSetAlpha(AlphaNone);
	NixSetZ(true,true);
}



bool ModeMaterial::open()
{
	if (!ed->allowTermination())
		return false;
	if (!ed->fileDialog(FDMaterial, false, false))
		return false;
	if (!data->load(ed->DialogFileComplete))
		return false;

	optimizeView();
	ed->setMode(mode_material);
	return true;
}



void ModeMaterial::onEnd()
{
	delete(AppearanceDialog);

	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->enable(false);
}



bool ModeMaterial::saveAs()
{
	if (ed->fileDialog(FDMaterial, true, false))
		return data->save(ed->DialogFileComplete);
	return false;
}



void ModeMaterial::onStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->addItem(L("new"),dir + "new.png","new");
	t->addItem(L("open"),dir + "open.png","open");
	t->addItem(L("save"),dir + "save.png","save");
	t->addSeparator();
	t->addItem(L("undo"),dir + "undo.png","undo");
	t->addItem(L("redo"),dir + "redo.png","redo");
	t->enable(true);
	t->configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->enable(false);
	multi_view->allow_rect = false;

	AppearanceDialog = new MaterialPropertiesDialog(ed, data);

	UpdateShape();
}

void ModeMaterial::SetShapeType(const string &type)
{
	shape_type = type;
	HuiConfig.setStr("MaterialShapeType", shape_type);
	UpdateShape();
}

void ModeMaterial::SetShapeSmooth(bool smooth)
{
	shape_smooth = smooth;
	HuiConfig.setBool("MaterialShapeSmooth", shape_smooth);
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
	onUpdateMenu();
	if (needs_opt_view)
		optimizeView();
}

bool ModeMaterial::optimizeView()
{
	multi_view->ResetView();
	if (geo){
		vector min, max;
		geo->GetBoundingBox(min, max);
		multi_view->SetViewBox(min, max);
	}
	return true;
}

void ModeMaterial::onUpdateMenu()
{
	ed->check("material_shape_smooth", shape_smooth);
	ed->check("material_shape_cube", shape_type == "cube");
	ed->check("material_shape_ball", shape_type == "ball");
	ed->check("material_shape_torus", shape_type == "torus");
	ed->check("material_shape_torusknot", shape_type == "torusknot");
	ed->check("material_shape_teapot", shape_type == "teapot");
	ed->check("material_shape_icosahedron", shape_type == "icosahedron");
}




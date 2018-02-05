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

	MaterialVB[1] = nix::vb_temp;
	for (int i=2;i<=MATERIAL_MAX_TEXTURES;i++)
		MaterialVB[i] = new nix::VertexBuffer(i);

	shape_type = hui::Config.getStr("MaterialShapeType", "teapot");
	shape_smooth = hui::Config.getBool("MaterialShapeSmooth", true);
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



void ModeMaterial::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		data->UpdateTextures();
		if (data->appearance.shader)
			data->appearance.shader->unref();
		data->appearance.shader = data->appearance.get_shader();
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

	nix::Draw3D(MaterialVB[max(data->appearance.texture_files.num, 1)]);


	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(true,true);
}



bool ModeMaterial::open()
{
	if (!ed->allowTermination())
		return false;
	if (!ed->fileDialog(FD_MATERIAL, false, false))
		return false;
	if (!data->load(ed->dialog_file_complete))
		return false;

	optimizeView();
	ed->setMode(mode_material);
	return true;
}



void ModeMaterial::onEnd()
{
	delete(AppearanceDialog);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(false);
}



bool ModeMaterial::saveAs()
{
	if (ed->fileDialog(FD_MATERIAL, true, false))
		return data->save(ed->dialog_file_complete);
	return false;
}



void ModeMaterial::onStart()
{
	string dir = "icons/toolbar/";
	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->addItem(L("", "new"),dir + "new.png","new");
	t->addItem(L("", "open"),dir + "open.png","open");
	t->addItem(L("", "save"),dir + "save.png","save");
	t->addSeparator();
	t->addItem(L("", "undo"),dir + "undo.png","undo");
	t->addItem(L("", "redo"),dir + "redo.png","redo");
	t->enable(true);
	t->configure(false,true);
	t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);


	multi_view->setAllowSelect(false);

	AppearanceDialog = new MaterialPropertiesDialog(ed, data);
	ed->embed(AppearanceDialog, "root-table", 1, 0);

	UpdateShape();
}

void ModeMaterial::SetShapeType(const string &type)
{
	shape_type = type;
	hui::Config.setStr("MaterialShapeType", shape_type);
	UpdateShape();
}

void ModeMaterial::SetShapeSmooth(bool smooth)
{
	shape_smooth = smooth;
	hui::Config.setBool("MaterialShapeSmooth", shape_smooth);
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
		geo = new GeometryTorusKnot(v_0, e_z, MATERIAL_RADIUS1, 40, 22, 2, 5, 128, 32);
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
		geo->smoothen();

	for (int i=1;i<=MATERIAL_MAX_TEXTURES;i++){
		nix::VertexBuffer *vb = MaterialVB[i];
		vb->clear();
		geo->build(vb);
	}
	onUpdateMenu();
	if (needs_opt_view)
		optimizeView();
}

bool ModeMaterial::optimizeView()
{
	multi_view->resetView();
	if (geo){
		vector min, max;
		geo->getBoundingBox(min, max);
		multi_view->setViewBox(min, max);
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




/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "../../Storage/Storage.h"
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

	for (int i=1; i<=MATERIAL_MAX_TEXTURES/2; i++) {
		string f = "3f,3fn";
		for (int j=0; j<i; j++)
			f += ",2f";
		MaterialVB[i] = new nix::VertexBuffer(f);
	}

	shape_type = hui::Config.get_str("MaterialShapeType", "teapot");
	shape_smooth = hui::Config.get_bool("MaterialShapeSmooth", true);
}

ModeMaterial::~ModeMaterial() {
	if (geo)
		delete(geo);
}

void ModeMaterial::_new()
{
	data->reset();
	optimize_view();
	ed->set_mode(mode_material);
}



bool ModeMaterial::save() {
	return storage->auto_save(data);
}



void ModeMaterial::on_draw() {
}



void ModeMaterial::on_update(Observable *o, const string &message) {
	if (o == data) {
		data->UpdateTextures();
		if (data->appearance.shader)
			data->appearance.shader->unref();
		data->appearance.shader = data->appearance.get_shader();
	}
}



void ModeMaterial::on_command(const string & id) {
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		save_as();

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


void ModeMaterial::on_draw_win(MultiView::Window *win) {
	data->ApplyForRendering();

	nix::DrawTriangles(MaterialVB[max(data->appearance.texture_files.num, 1)]);


	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(true,true);
}



bool ModeMaterial::open() {
	if (!storage->open(data))
		return false;

	optimize_view();
	ed->set_mode(mode_material);
	return true;
}



void ModeMaterial::on_end() {
	ed->set_side_panel(nullptr);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(false);
}



bool ModeMaterial::save_as() {
	return storage->save_as(data);
}



void ModeMaterial::on_start() {
	ed->toolbar[hui::TOOLBAR_TOP]->set_by_id("material-toolbar");
	auto t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);


	multi_view->set_allow_select(false);

	AppearanceDialog = new MaterialPropertiesDialog(ed, data);
	ed->set_side_panel(AppearanceDialog);

	UpdateShape();
}

void ModeMaterial::SetShapeType(const string &type) {
	shape_type = type;
	hui::Config.set_str("MaterialShapeType", shape_type);
	UpdateShape();
}

void ModeMaterial::SetShapeSmooth(bool smooth) {
	shape_smooth = smooth;
	hui::Config.set_bool("MaterialShapeSmooth", shape_smooth);
	UpdateShape();
}

void ModeMaterial::UpdateShape() {
	bool needs_opt_view = !geo;
	if (geo)
		delete(geo);
	if (shape_type == "torus")
		geo = new GeometryTorus(v_0, vector::EZ, MATERIAL_RADIUS1, MATERIAL_RADIUS2, MATERIAL_NUMX, MATERIAL_NUMY);
	else if (shape_type == "torusknot")
		geo = new GeometryTorusKnot(v_0, vector::EZ, MATERIAL_RADIUS1, 40, 22, 2, 5, 128, 32);
	else if (shape_type == "teapot")
		geo = new GeometryTeapot(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "cube")
		geo = new GeometryCube(-vector(1,1,1) * MATERIAL_RADIUS1/2, vector::EX * MATERIAL_RADIUS1, vector::EY * MATERIAL_RADIUS1, vector::EZ * MATERIAL_RADIUS1, 1, 1, 1);
		//geo = new ModelGeometryPlatonic(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "icosahedron")
		geo = new GeometryPlatonic(v_0, MATERIAL_RADIUS1, 20);
	else //if (shape_type == "ball")
		geo = new GeometryBall(v_0, MATERIAL_RADIUS1, 32, 16);
	if (shape_smooth)
		geo->smoothen();

	for (int i=1; i<=MATERIAL_MAX_TEXTURES; i++) {
		geo->build(MaterialVB[i]);
	}
	on_update_menu();
	if (needs_opt_view)
		optimize_view();
}

bool ModeMaterial::optimize_view() {
	multi_view->reset_view();
	if (geo) {
		vector min, max;
		geo->get_bounding_box(min, max);
		multi_view->set_view_box(min, max);
	}
	return true;
}

void ModeMaterial::on_update_menu() {
	ed->check("material_shape_smooth", shape_smooth);
	ed->check("material_shape_cube", shape_type == "cube");
	ed->check("material_shape_ball", shape_type == "ball");
	ed->check("material_shape_torus", shape_type == "torus");
	ed->check("material_shape_torusknot", shape_type == "torusknot");
	ed->check("material_shape_teapot", shape_type == "teapot");
	ed->check("material_shape_icosahedron", shape_type == "icosahedron");
}




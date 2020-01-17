/*
 * ModeModelMeshMaterial.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshMaterial.h"
#include "../Dialog/ModelMaterialDialog.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../MultiView/MultiView.h"
#include "../../../Edward.h"

ModeModelMeshMaterial *mode_model_mesh_material = NULL;

ModeModelMeshMaterial::ModeModelMeshMaterial(ModeBase *_parent) :
			Mode<DataModel>("ModelMeshMaterial", _parent, ed->multi_view_3d, "menu_model"),
		Observable("ModelMeshMaterial") {
	dialog = nullptr;
}

ModeModelMeshMaterial::~ModeModelMeshMaterial() {
}

void ModeModelMeshMaterial::on_start() {

	dialog = new ModelMaterialDialog(data);
	ed->embed(dialog, "root-table", 1, 0);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
	multi_view->set_allow_action(false);

	// enter
	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	mode_model->allow_selection_modes(false);
}

void ModeModelMeshMaterial::on_end() {
	delete dialog;
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	mode_model->allow_selection_modes(true);
}

void ModeModelMeshMaterial::on_set_multi_view() {
	parent->on_set_multi_view();
}

void ModeModelMeshMaterial::on_draw() {
	parent->on_draw();
}

void ModeModelMeshMaterial::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}

void ModeModelMeshMaterial::on_selection_change() {
	parent->on_selection_change();
}

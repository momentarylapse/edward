/*
 * ModeModelMeshMaterial.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshMaterial.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../dialog/ModelMaterialDialog.h"
#include "../../../multiview/MultiView.h"
#include "../../../EdwardWindow.h"

ModeModelMeshMaterial::ModeModelMeshMaterial(ModeModelMesh *_parent, MultiView::MultiView *mv) :
			Mode<ModeModelMesh, DataModel>(_parent->ed, "ModelMeshMaterial", _parent, mv, "menu_model") {
	dialog = nullptr;
}

void ModeModelMeshMaterial::on_start() {

	dialog = new ModelMaterialDialog(data);
	ed->set_side_panel(dialog);

	auto *t = ed->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);
	multi_view->set_allow_action(false);

	// enter
	parent->set_selection_mode(parent->selection_mode_polygon);
	ed->mode_model->allow_selection_modes(false);
}

void ModeModelMeshMaterial::on_end() {
	ed->set_side_panel(nullptr);
	ed->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->set_allow_action(true);
	ed->mode_model->allow_selection_modes(true);
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

/*
 * ModeModelMeshPaint.cpp
 *
 *  Created on: 16.01.2020
 *      Author: michi
 */

#include "ModeModelMeshPaint.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../../../MultiView/MultiView.h"
#include "../../../Edward.h"

ModeModelMeshPaint *mode_model_mesh_paint = NULL;

ModeModelMeshPaint::ModeModelMeshPaint(ModeBase *_parent) :
			Mode<DataModel>("ModelMeshPaint", _parent, ed->multi_view_3d, "menu_model"),
		Observable("ModelMeshPaint") {
//	dialog = nullptr;
}

ModeModelMeshPaint::~ModeModelMeshPaint() {
}

void ModeModelMeshPaint::on_start() {

//	dialog = new ModelMaterialDialog(data);
//	ed->embed(dialog, "root-table", 1, 0);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
	multi_view->setAllowAction(false);

	// enter
	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_polygon);
	mode_model->allowSelectionModes(false);
}

void ModeModelMeshPaint::on_end() {
//	delete dialog;
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // back to mesh....ARGH

	multi_view->setAllowAction(true);
	mode_model->allowSelectionModes(true);
}

void ModeModelMeshPaint::on_set_multi_view() {
	parent->on_set_multi_view();
}

void ModeModelMeshPaint::on_draw() {
	parent->on_draw();
}

void ModeModelMeshPaint::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}

void ModeModelMeshPaint::on_selection_change() {
	parent->on_selection_change();
}

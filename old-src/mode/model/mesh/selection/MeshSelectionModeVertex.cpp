/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeVertex.h"
#include "MeshSelectionModePolygon.h"
#include "../ModeModelMesh.h"
#include "../../skeleton/ModeModelSkeleton.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include "../../../../Session.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../lib/nix/nix.h"

MeshSelectionModeVertex::MeshSelectionModeVertex(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{}

void MeshSelectionModeVertex::on_start() {
}

void MeshSelectionModeVertex::on_end() {
}



void MeshSelectionModeVertex::on_draw_win(MultiView::Window *win) {
	parent->draw_effects(win);
}


void MeshSelectionModeVertex::on_update_selection() {
	data->edit_mesh->selection_from_vertices();
}

void MeshSelectionModeVertex::on_view_stage_change() {
	for (auto &p: data->mesh->polygon)
		if (p.is_selected)
			p.view_stage = multi_view->view_stage;
		else
			p.view_stage = min(p.view_stage, multi_view->view_stage);
	for (auto &e: data->mesh->edge)
		if (e.is_selected)
			e.view_stage = multi_view->view_stage;
		else
			e.view_stage = min(e.view_stage, multi_view->view_stage);

}

void MeshSelectionModeVertex::update_multi_view() {
	multi_view->clear_data(data);
	multi_view->add_data(MVD_MODEL_VERTEX,
			data->edit_mesh->vertex,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

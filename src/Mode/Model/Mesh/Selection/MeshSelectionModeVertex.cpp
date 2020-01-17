/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeVertex.h"

#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../ModeModelMesh.h"
#include "../../Skeleton/ModeModelSkeleton.h"
#include "../../../../lib/nix/nix.h"
#include "MeshSelectionModePolygon.h"

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


void MeshSelectionModeVertex::update_selection() {
	data->selectionFromVertices();
}

void MeshSelectionModeVertex::update_multi_view() {
	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(	MVD_MODEL_VERTEX,
			data->show_vertices,
			NULL,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

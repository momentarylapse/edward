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
{
}

void MeshSelectionModeVertex::onStart()
{
}

void MeshSelectionModeVertex::onEnd()
{
}



void MeshSelectionModeVertex::onDrawWin(MultiView::Window *win)
{
	parent->draw_effects(win);
}



void MeshSelectionModeVertex::onDraw()
{
}

void MeshSelectionModeVertex::updateSelection()
{
	data->selectionFromVertices();
}

void MeshSelectionModeVertex::updateMultiView()
{
	multi_view->clearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->addData(	MVD_MODEL_VERTEX,
			data->show_vertices,
			NULL,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

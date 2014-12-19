/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeVertex.h"

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../../../lib/nix/nix.h"
#include "MeshSelectionModePolygon.h"

MeshSelectionModeVertex::MeshSelectionModeVertex(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModeVertex::onStart()
{
	multi_view->allow_rect = true;
}

void MeshSelectionModeVertex::onEnd()
{
}



void MeshSelectionModeVertex::onDrawWin(MultiView::Window *win)
{
	parent->drawEffects(win);
}



void MeshSelectionModeVertex::onDraw()
{
}

void MeshSelectionModeVertex::updateSelection()
{
	data->SelectionFromVertices();
}

void MeshSelectionModeVertex::updateMultiView()
{
	multi_view->ClearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->AddData(	MVD_MODEL_VERTEX,
			data->vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
}

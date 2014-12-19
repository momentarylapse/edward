/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeSurface.h"

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "ModeModelMesh.h"
#include "MeshSelectionModePolygon.h"


MeshSelectionModeSurface::MeshSelectionModeSurface(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModeSurface::updateSelection()
{
	data->SelectionFromSurfaces();
}

void MeshSelectionModeSurface::updateMultiView()
{
	multi_view->ClearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->AddData(	MVD_MODEL_SURFACE,
			data->surface,
			data,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
}

void MeshSelectionModeSurface::onDrawWin(MultiView::Window *win)
{
}



void MeshSelectionModeSurface::onEnd()
{
}

bool ModelSurface::hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data)
{
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].hover(win, m, tp, z, user_data))
			return true;
	return false;
}

bool ModelSurface::inRect(MultiView::Window *win, rect &r, void *user_data)
{
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].inRect(win, r, user_data))
			return true;
	return false;
}



void MeshSelectionModeSurface::onStart()
{
	multi_view->allow_rect = true;
}



void MeshSelectionModeSurface::onDraw()
{
}



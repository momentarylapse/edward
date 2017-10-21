/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeSurface.h"

#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../ModeModelMesh.h"
#include "MeshSelectionModePolygon.h"
#include "../../ModeModel.h"


MeshSelectionModeSurface::MeshSelectionModeSurface(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModeSurface::updateSelection()
{
	data->selectionFromSurfaces();
}

void MeshSelectionModeSurface::updateMultiView()
{
	multi_view->clearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->addData(	MVD_MODEL_SURFACE,
			data->surface,
			data,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

void MeshSelectionModeSurface::onDrawWin(MultiView::Window *win)
{
	if ((multi_view->hover.index < 0) || (multi_view->hover.type != MVD_MODEL_SURFACE))
		return;

	parent->vb_hover->clear();


	ModelSurface &s = data->surface[multi_view->hover.index];
	for (ModelPolygon &p: s.polygon)
		p.addToVertexBuffer(data->vertex, parent->vb_hover, 1);


	nix::SetWire(false);
	nix::SetOffset(1.0f);
	mode_model->setMaterialHover();
	nix::Draw3D(parent->vb_hover);
	nix::SetMaterial(White,White,Black,0,Black);
	nix::SetAlpha(AlphaNone);
	nix::SetOffset(0);
	nix::SetWire(multi_view->wire_mode);
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
}



void MeshSelectionModeSurface::onDraw()
{
}



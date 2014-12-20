/*
 * ModeModelMeshBevelEdges.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelEdges.h"
#include "../ModeModelMesh.h"
#include "../../../../Action/Model/Mesh/Edge/ActionModelBevelEdges.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

ModeModelMeshBevelEdges::ModeModelMeshBevelEdges(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshBevelEdges", _parent)
{
	message = _("Radius skalieren [Linke Maustaste = fertig]");


	data->getSelectionState(selection);

	// find maximal radius
	rad_max = -1;
	foreach(ModelSurface &s, data->surface)
		foreach(ModelEdge &e, s.edge)
			if ((data->vertex[e.vertex[0]].is_selected) or (data->vertex[e.vertex[1]].is_selected)){
			float l = (data->vertex[e.vertex[0]].pos - data->vertex[e.vertex[1]].pos).length();
			if ((data->vertex[e.vertex[0]].is_selected) and (data->vertex[e.vertex[1]].is_selected))
				l /= 2;
			if ((l < rad_max) or (rad_max < 0))
				rad_max = l;
		}

	radius = rad_max / 4;

	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();
}

ModeModelMeshBevelEdges::~ModeModelMeshBevelEdges()
{
}

void ModeModelMeshBevelEdges::onEnd()
{
	data->action_manager->clearPreview();
}

void ModeModelMeshBevelEdges::onMouseMove()
{
	data->action_manager->clearPreview();

	radius += (HuiGetEvent()->dx) / multi_view->cam.zoom;
	radius = clampf(radius, rad_max * 0.001f, rad_max);

	data->setSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();
}

void ModeModelMeshBevelEdges::onLeftButtonDown()
{
	data->action_manager->clearPreview();

	data->setSelectionState(selection);
	data->bevelSelectedEdges(radius);
	abort();
}

void ModeModelMeshBevelEdges::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	NixEnableLighting(false);
	ed->drawStr(100, 100, f2s(radius, 3));
}



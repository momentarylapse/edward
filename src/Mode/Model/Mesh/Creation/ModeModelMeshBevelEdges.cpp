/*
 * ModeModelMeshBevelEdges.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelEdges.h"
#include "../../../../Action/Model/Mesh/Edge/ActionModelBevelEdges.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

#define INTERACTIVE

ModeModelMeshBevelEdges::ModeModelMeshBevelEdges(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshBevelEdges", _parent)
{
	message = _("Radius skalieren [Linke Maustaste = fertig]");


	data->GetSelectionState(selection);

	// find maximal radius
	rad_max = -1;
	foreach(ModelSurface &s, data->Surface)
		foreach(ModelEdge &e, s.Edge)
			if ((data->Vertex[e.Vertex[0]].is_selected) or (data->Vertex[e.Vertex[1]].is_selected)){
			float l = (data->Vertex[e.Vertex[0]].pos - data->Vertex[e.Vertex[1]].pos).length();
			if ((data->Vertex[e.Vertex[0]].is_selected) and (data->Vertex[e.Vertex[1]].is_selected))
				l /= 2;
			if ((l < rad_max) or (rad_max < 0))
				rad_max = l;
		}

	radius = rad_max / 4;
#ifdef INTERACTIVE
	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();
#endif
}

ModeModelMeshBevelEdges::~ModeModelMeshBevelEdges()
{
}

void ModeModelMeshBevelEdges::onEnd()
{
#ifdef INTERACTIVE
	data->action_manager->clearPreview();
#endif
}

void ModeModelMeshBevelEdges::onMouseMove()
{
#ifdef INTERACTIVE
	data->action_manager->clearPreview();
#endif

	radius += (HuiGetEvent()->dx) / multi_view->cam.zoom;
	radius = clampf(radius, rad_max * 0.001f, rad_max);

#ifdef INTERACTIVE

	data->SetSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelBevelEdges(radius)))
		abort();
#endif
}

void ModeModelMeshBevelEdges::onLeftButtonDown()
{
#ifdef INTERACTIVE
	data->action_manager->clearPreview();
#endif

	data->SetSelectionState(selection);
	data->BevelSelectedEdges(radius);
	abort();
}

void ModeModelMeshBevelEdges::onDrawWin(MultiView::Window *win)
{
#ifndef INTERACTIVE
	mode_model->SetMaterialCreation();
	foreach(ModelVertex &v, data->Vertex)
		if (v.is_selected)
			FxDrawBall(v.pos, radius, 16,32);
#endif
	NixEnableLighting(false);
	ed->drawStr(100, 100, f2s(radius, 3));
}



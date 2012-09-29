/*
 * ModeModelMeshBevelVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelVertices.h"
#include "../../../../Action/Model/Mesh/Vertex/ActionModelBevelVertices.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../lib/x/x.h"

#define INTERACTIVE

ModeModelMeshBevelVertices::ModeModelMeshBevelVertices(Mode *_parent) :
	ModeCreation("ModelMeshBevelVertices", _parent)
{
	data = (DataModel*)_parent->GetData();

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

	radius = rad_max / 2;
}

ModeModelMeshBevelVertices::~ModeModelMeshBevelVertices()
{
}

void ModeModelMeshBevelVertices::OnEnd()
{
#ifdef INTERACTIVE
	data->action_manager->ClearPreview();
#endif
}

void ModeModelMeshBevelVertices::OnMouseMove()
{
#ifdef INTERACTIVE
	data->action_manager->ClearPreview();
#endif

	radius += (HuiGetEvent()->dx) / multi_view->zoom;
	radius = clampf(radius, rad_max * 0.001f, rad_max);

#ifdef INTERACTIVE

	data->SetSelectionState(selection);
	if (!data->action_manager->Preview(new ActionModelBevelVertices(radius)))
		Abort();
#endif
}

void ModeModelMeshBevelVertices::OnLeftButtonDown()
{
#ifdef INTERACTIVE
	data->action_manager->ClearPreview();
#endif

	data->SetSelectionState(selection);
	data->BevelSelectedVertices(radius);
	Abort();
}

void ModeModelMeshBevelVertices::OnDrawWin(int win, irect dest)
{
#ifndef INTERACTIVE
	mode_model->SetMaterialCreation();
	foreach(ModelVertex &v, data->Vertex)
		if (v.is_selected)
			FxDrawBall(v.pos, radius, 16,32);
#endif
	NixEnableLighting(false);
	ed->DrawStr(100, 100, f2s(radius, 3));
}



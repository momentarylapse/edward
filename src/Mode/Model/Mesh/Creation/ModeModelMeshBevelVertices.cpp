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

//#define INTERACTIVE

ModeModelMeshBevelVertices::ModeModelMeshBevelVertices(Mode *_parent) :
	ModeCreation("ModelMeshBevelVertices", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Radius skalieren [Shift + Return]");


	data->GetSelectionState(selection);

	radius = 2 / multi_view->zoom;
#ifdef INTERACTIVE
	a = new ActionModelBevelVertices(radius);
	a->execute_logged(data);
	data->Notify("Changed");
#endif
}

ModeModelMeshBevelVertices::~ModeModelMeshBevelVertices()
{
}

void ModeModelMeshBevelVertices::OnEnd()
{
#ifdef INTERACTIVE
	if (a){
		a->undo_logged(data);
		data->Notify("Changed");
		delete(a);
	}
#endif
}

void ModeModelMeshBevelVertices::OnMouseMove()
{
#ifdef INTERACTIVE
	a->undo_logged(data);
	data->Notify("Changed");
	delete(a);
#endif

	radius += (HuiGetEvent()->dx) / multi_view->zoom;

#ifdef INTERACTIVE
	data->SetSelectionState(selection);
	a = new ActionModelBevelVertices(fabs(radius));
	a->execute_logged(data);
	data->Notify("Changed");
#endif
}

void ModeModelMeshBevelVertices::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN)
		if (data->GetNumSelectedVertices() > 0){
#ifdef INTERACTIVE
			a->undo_logged(data);
			data->Notify("Changed");
			delete(a);
			a = NULL;
#endif

			data->SetSelectionState(selection);
			data->BevelSelectedVertices(radius);
			Abort();
		}
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
	ed->DrawStr(100, 100, f2s(fabs(radius), 3));
}



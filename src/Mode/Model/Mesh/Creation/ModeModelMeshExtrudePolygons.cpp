/*
 * ModeModelMeshExtrudePolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ModeModelMeshExtrudePolygons.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Triangle/ActionModelExtrudePolygons.h"

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(Mode *_parent) :
	ModeCreation("ModelMeshExtrudePolygons", _parent)
{
	data = (DataModel*)_parent->GetData();

	data->GetSelectionState(selection);

	offset = 20.0f / multi_view->zoom;
	a = new ActionModelExtrudePolygons(offset);
	a->execute_logged(data);

	message = _("Extrudieren: Offset durch Maus, Linke Taste = fertig");
}

ModeModelMeshExtrudePolygons::~ModeModelMeshExtrudePolygons()
{
}

void ModeModelMeshExtrudePolygons::OnEnd()
{
	if (a){
		a->undo_logged(data);
		delete(a);
	}
}

void ModeModelMeshExtrudePolygons::OnLeftButtonDown()
{
	a->undo_logged(data);
	delete(a);

	data->SetSelectionState(selection);
	data->Execute(new ActionModelExtrudePolygons(offset));

	a = NULL;
	Abort();
}

void ModeModelMeshExtrudePolygons::OnMouseMove()
{
	a->undo_logged(data);
	delete(a);

	offset += (HuiGetEvent()->dx) / multi_view->zoom;
	data->SetSelectionState(selection);
	a = new ActionModelExtrudePolygons(offset);
	a->execute_logged(data);
}

void ModeModelMeshExtrudePolygons::OnDrawWin(int win, irect dest)
{
	NixEnableLighting(false);
	ed->DrawStr(100, 100, f2s(offset, 3));
}

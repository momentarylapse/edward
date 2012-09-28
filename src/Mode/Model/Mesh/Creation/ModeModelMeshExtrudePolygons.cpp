/*
 * ModeModelMeshExtrudePolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ModeModelMeshExtrudePolygons.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.h"

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(Mode *_parent) :
	ModeCreation("ModelMeshExtrudePolygons", _parent)
{
	data = (DataModel*)_parent->GetData();

	data->GetSelectionState(selection);

	offset = 20.0f / multi_view->zoom;
	Preview();

	message = _("Extrudieren: Offset durch Maus, Linke Taste = fertig");
}

ModeModelMeshExtrudePolygons::~ModeModelMeshExtrudePolygons()
{
}

void ModeModelMeshExtrudePolygons::OnEnd()
{
	CleanUp();
}

void ModeModelMeshExtrudePolygons::OnLeftButtonDown()
{
	CleanUp();

	data->SetSelectionState(selection);
	data->Execute(new ActionModelExtrudePolygons(offset));

	Abort();
}

void ModeModelMeshExtrudePolygons::OnMouseMove()
{
	CleanUp();

	offset += (HuiGetEvent()->dx) / multi_view->zoom;
	Preview();
}

void ModeModelMeshExtrudePolygons::OnDrawWin(int win, irect dest)
{
	NixEnableLighting(false);
	ed->DrawStr(100, 100, f2s(offset, 3));
}

void ModeModelMeshExtrudePolygons::Preview()
{
	data->SetSelectionState(selection);
	a = new ActionModelExtrudePolygons(offset);
	try{
		a->execute_logged(data);
	}catch(ActionException &e){
		a->abort(data);
		delete(a);
		a = NULL;
		Abort();
		ed->ErrorBox(e.message);
	}
}

void ModeModelMeshExtrudePolygons::CleanUp()
{
	if (a){
		a->undo_logged(data);
		delete(a);
		a = NULL;
	}
}


/*
 * ModeModelMeshCreateVertex.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateVertex.h"
#include "../../../../Edward.h"

ModeModelMeshCreateVertex::ModeModelMeshCreateVertex(Mode *_parent) :
	ModeCreation(_parent)
{
	name = "ModelMeshCreateVertex";
	data = (DataModel*)_parent->GetData();

	message = _("neue Punkte setzen");
}

ModeModelMeshCreateVertex::~ModeModelMeshCreateVertex()
{
}


void ModeModelMeshCreateVertex::OnLeftButtonDown()
{
	data->AddVertex(multi_view->GetCursor3d());
	//Abort();
}

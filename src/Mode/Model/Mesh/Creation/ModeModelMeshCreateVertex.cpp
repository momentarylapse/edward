/*
 * ModeModelMeshCreateVertex.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateVertex.h"
#include "../../../../Edward.h"

ModeModelMeshCreateVertex::ModeModelMeshCreateVertex(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateVertex";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("neue Punkte setzen");
}

ModeModelMeshCreateVertex::~ModeModelMeshCreateVertex()
{
}


void ModeModelMeshCreateVertex::OnLeftButtonDown()
{
	data->AddVertex(multi_view->GetCursor3d());
	//ed->SetCreationMode(NULL);
}

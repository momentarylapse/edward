/*
 * ModeModelMeshCreateVertex.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateVertex.h"
#include "../ModeModelMesh.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

ModeModelMeshCreateVertex::ModeModelMeshCreateVertex(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateVertex", _parent)
{
	message = _("neue Punkte setzen");

	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);
}

ModeModelMeshCreateVertex::~ModeModelMeshCreateVertex()
{
}


void ModeModelMeshCreateVertex::onLeftButtonDown()
{
	data->AddVertex(multi_view->GetCursor3d());
	//Abort();
}

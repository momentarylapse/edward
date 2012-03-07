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

void ModeModelMeshCreateVertex::End()
{
}



void ModeModelMeshCreateVertex::OnMouseMove()
{
}



void ModeModelMeshCreateVertex::Start()
{
}



void ModeModelMeshCreateVertex::OnRightButtonUp()
{
}



void ModeModelMeshCreateVertex::OnKeyDown()
{
}



void ModeModelMeshCreateVertex::PostDraw()
{
}



void ModeModelMeshCreateVertex::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateVertex::OnLeftButtonDown()
{
	data->AddVertex(multi_view->GetCursor3d());
	ed->SetCreationMode(NULL);
}



void ModeModelMeshCreateVertex::OnLeftButtonUp()
{
}



void ModeModelMeshCreateVertex::PostDrawWin(int win, irect dest)
{
}



void ModeModelMeshCreateVertex::OnCommand(const string & id)
{
}



void ModeModelMeshCreateVertex::OnKeyUp()
{
}



void ModeModelMeshCreateVertex::OnRightButtonDown()
{
}



void ModeModelMeshCreateVertex::OnMiddleButtonUp()
{
}



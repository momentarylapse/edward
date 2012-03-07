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
	msg_write(p2s(this));
	msg_write(p2s(parent));
	msg_write(parent->name);
	msg_write(p2s(parent->multi_view));
	data->AddVertex(parent->multi_view->GetCursor3d());
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



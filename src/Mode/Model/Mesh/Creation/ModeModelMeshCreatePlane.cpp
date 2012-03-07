/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreatePlane.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddPlane.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreatePlane";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("ebene...");
}

ModeModelMeshCreatePlane::~ModeModelMeshCreatePlane()
{
	// TODO Auto-generated destructor stub
}

void ModeModelMeshCreatePlane::PostDraw()
{
}



void ModeModelMeshCreatePlane::End()
{
}



void ModeModelMeshCreatePlane::Start()
{
}



void ModeModelMeshCreatePlane::OnRightButtonUp()
{
}



void ModeModelMeshCreatePlane::OnCommand(const string & id)
{
}



void ModeModelMeshCreatePlane::OnKeyDown()
{
}



void ModeModelMeshCreatePlane::OnRightButtonDown()
{
}



void ModeModelMeshCreatePlane::OnMiddleButtonUp()
{
}



void ModeModelMeshCreatePlane::OnMiddleButtonDown()
{
}



void ModeModelMeshCreatePlane::OnLeftButtonDown()
{
}



void ModeModelMeshCreatePlane::OnLeftButtonUp()
{
}



void ModeModelMeshCreatePlane::OnKeyUp()
{
}



void ModeModelMeshCreatePlane::PostDrawWin(int win, irect dest)
{
}



void ModeModelMeshCreatePlane::OnMouseMove()
{
}



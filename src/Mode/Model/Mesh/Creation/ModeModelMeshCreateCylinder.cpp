/*
 * ModeModelMeshCreateCylinder.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinder.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddCylinder.h"

ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateCylinder";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("zylinder...");
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
	// TODO Auto-generated destructor stub
}

void ModeModelMeshCreateCylinder::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateCylinder::PostDraw()
{
}



void ModeModelMeshCreateCylinder::OnMiddleButtonUp()
{
}



void ModeModelMeshCreateCylinder::End()
{
}



void ModeModelMeshCreateCylinder::OnKeyDown()
{
}



void ModeModelMeshCreateCylinder::OnCommand(const string & id)
{
}



void ModeModelMeshCreateCylinder::OnRightButtonDown()
{
}



void ModeModelMeshCreateCylinder::OnMouseMove()
{
}



void ModeModelMeshCreateCylinder::Start()
{
}



void ModeModelMeshCreateCylinder::OnRightButtonUp()
{
}



void ModeModelMeshCreateCylinder::OnLeftButtonDown()
{
}



void ModeModelMeshCreateCylinder::OnLeftButtonUp()
{
}



void ModeModelMeshCreateCylinder::PostDrawWin(int win, irect dest)
{
}



void ModeModelMeshCreateCylinder::OnKeyUp()
{
}



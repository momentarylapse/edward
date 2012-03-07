/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCube.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddCube.h"

ModeModelMeshCreateCube::ModeModelMeshCreateCube(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateCube";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("cube...");
}

ModeModelMeshCreateCube::~ModeModelMeshCreateCube()
{
	// TODO Auto-generated destructor stub
}

void ModeModelMeshCreateCube::PostDraw()
{
}



void ModeModelMeshCreateCube::OnLeftButtonDown()
{
}



void ModeModelMeshCreateCube::OnRightButtonUp()
{
}



void ModeModelMeshCreateCube::OnRightButtonDown()
{
}



void ModeModelMeshCreateCube::OnKeyDown()
{
}



void ModeModelMeshCreateCube::OnKeyUp()
{
}



void ModeModelMeshCreateCube::OnMiddleButtonUp()
{
}



void ModeModelMeshCreateCube::Start()
{
}



void ModeModelMeshCreateCube::End()
{
}



void ModeModelMeshCreateCube::OnMouseMove()
{
}



void ModeModelMeshCreateCube::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateCube::OnLeftButtonUp()
{
}



void ModeModelMeshCreateCube::OnCommand(const string & id)
{
}



void ModeModelMeshCreateCube::PostDrawWin(int win, irect dest)
{
}



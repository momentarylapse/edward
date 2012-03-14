/*
 * ModeModelMeshEdge.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshEdge.h"

ModeModelMeshEdge *mode_model_mesh_edge = NULL;

ModeModelMeshEdge::ModeModelMeshEdge(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshEdge";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}



ModeModelMeshEdge::~ModeModelMeshEdge()
{
}

void ModeModelMeshEdge::OnRightButtonUp()
{
}



void ModeModelMeshEdge::OnKeyDown()
{
}



void ModeModelMeshEdge::OnRightButtonDown()
{
}



void ModeModelMeshEdge::OnMiddleButtonDown()
{
}



void ModeModelMeshEdge::OnMiddleButtonUp()
{
}



void ModeModelMeshEdge::OnKeyUp()
{
}



void ModeModelMeshEdge::OnUpdateMenu()
{
}



void ModeModelMeshEdge::Draw()
{
}



void ModeModelMeshEdge::OnMouseMove()
{
}



void ModeModelMeshEdge::Start()
{
}



void ModeModelMeshEdge::OnUpdate(Observable *o)
{
}



void ModeModelMeshEdge::OnCommand(const string & id)
{
}



void ModeModelMeshEdge::OnLeftButtonDown()
{
}



void ModeModelMeshEdge::DrawWin(int win, irect dest)
{
}



void ModeModelMeshEdge::End()
{
}



void ModeModelMeshEdge::OnLeftButtonUp()
{
}



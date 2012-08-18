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

ModeModelMeshEdge::ModeModelMeshEdge(Mode *_parent, DataModel *_data) :
	Mode("ModelMeshEdge", _parent, _data, ed->multi_view_3d, "menu_model")
{
	data = _data;
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



void ModeModelMeshEdge::OnDraw()
{
}



void ModeModelMeshEdge::OnMouseMove()
{
}



void ModeModelMeshEdge::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
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



void ModeModelMeshEdge::OnDrawWin(int win, irect dest)
{
}



void ModeModelMeshEdge::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshEdge::OnLeftButtonUp()
{
}



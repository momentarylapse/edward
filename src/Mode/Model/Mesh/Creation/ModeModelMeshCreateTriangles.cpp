/*
 * ModeModelMeshCreateTriangles.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTriangles.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddTrianglesByOutline.h"

ModeModelMeshCreateTriangles::ModeModelMeshCreateTriangles(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateTriangles";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = format(_("Dreiecke w&ahlen: %d -> Shift + Return"), 0);
}

ModeModelMeshCreateTriangles::~ModeModelMeshCreateTriangles()
{
}

void ModeModelMeshCreateTriangles::OnLeftButtonUp()
{
}



void ModeModelMeshCreateTriangles::Start()
{
	foreach(data->Vertex, v)
		v.is_special = false;
}



void ModeModelMeshCreateTriangles::End()
{
	foreach(data->Vertex, v)
		v.is_special = false;
}



void ModeModelMeshCreateTriangles::OnRightButtonDown()
{
}



void ModeModelMeshCreateTriangles::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateTriangles::PostDrawWin(int win, irect dest)
{
}



void ModeModelMeshCreateTriangles::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		data->Execute(new ActionModelAddTrianglesByOutline(selection, data));
		ed->SetCreationMode(NULL);
	}
}



void ModeModelMeshCreateTriangles::OnKeyUp()
{
}



void ModeModelMeshCreateTriangles::OnMiddleButtonUp()
{
}



void ModeModelMeshCreateTriangles::OnLeftButtonDown()
{
	if (multi_view->Selected >= 0){
		selection.add(multi_view->Selected);
	}else{
		data->AddVertex(multi_view->GetCursor3d());
		selection.add(data->Vertex.num - 1);
	}
	data->Vertex[selection.back()].is_special = true;
	message = format(_("Dreiecke w&ahlen: %d ->Return"), selection.num);
}



void ModeModelMeshCreateTriangles::PostDraw()
{
}



void ModeModelMeshCreateTriangles::OnRightButtonUp()
{
}



void ModeModelMeshCreateTriangles::OnMouseMove()
{
}



void ModeModelMeshCreateTriangles::OnCommand(const string & id)
{
}



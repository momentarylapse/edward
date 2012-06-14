/*
 * ModeModelMeshCreateTriangles.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTriangles.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Triangle/ActionModelAddTrianglesByOutline.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshCreateTriangles::ModeModelMeshCreateTriangles(Mode *_parent) :
	ModeCreation(_parent)
{
	name = "ModelMeshCreateTriangles";
	data = (DataModel*)_parent->GetData();

	message = format(_("Dreiecke w&ahlen: %d -> Shift + Return"), 0);
}

ModeModelMeshCreateTriangles::~ModeModelMeshCreateTriangles()
{
}


void ModeModelMeshCreateTriangles::OnStart()
{
	foreach(data->Vertex, v)
		v.is_special = false;
}



void ModeModelMeshCreateTriangles::OnEnd()
{
	foreach(data->Vertex, v)
		v.is_special = false;
}


void ModeModelMeshCreateTriangles::OnDrawWin(int win, irect dest)
{
	for (int i=1;i<selection.num;i++){
		vector pa = multi_view->VecProject(data->Vertex[selection[i - 1]].pos, win);
		vector pb = multi_view->VecProject(data->Vertex[selection[i    ]].pos, win);
		if ((pa.z >= 0) and (pa.z < 1) and (pb.z >= 0) and (pb.z <= 1))
			NixDrawLine(pa.x, pa.y, pb.x, pb.y, Green, 0);
	}
}



void ModeModelMeshCreateTriangles::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		data->Execute(new ActionModelAddTrianglesByOutline(selection, data));
		Abort();
	}
}


void ModeModelMeshCreateTriangles::OnLeftButtonDown()
{
	if (multi_view->Selected >= 0){
		// closed loop -> done
		if (selection.num > 0)
			if (multi_view->Selected == selection[0]){
				data->Execute(new ActionModelAddTrianglesByOutline(selection, data));
				Abort();
				return;
			}

		// consistency?
		foreachi(selection, s, i)
			if (s == multi_view->Selected)
				if (i > 0){
					ed->SetMessage(_("keine doppelten Punkte erlaubt!"));
					Abort();
					return;
				}

		// all ok -> add
		selection.add(multi_view->Selected);

	}else{
		data->AddVertex(multi_view->GetCursor3d());
		selection.add(data->Vertex.num - 1);
	}
	data->Vertex[selection.back()].is_special = true;
	message = format(_("Dreiecke w&ahlen: %d -> Shift + Return"), selection.num);
}


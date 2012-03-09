/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
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
	pos_chosen = false;
	pos2_chosen = false;
	for (int i=0;i<3;i++)
		length[i] = v0;
}

ModeModelMeshCreateCube::~ModeModelMeshCreateCube()
{
}

void ModeModelMeshCreateCube::PostDraw()
{
}



void ModeModelMeshCreateCube::OnLeftButtonDown()
{
	if (pos_chosen){
		if (pos2_chosen){

			data->Execute(new ActionModelAddCube(pos, length[0], length[1], length[2], data->Vertex.num));

			ed->SetCreationMode(NULL);
		}else{
			if (multi_view->Selected >= 0)
				pos2 = data->Vertex[multi_view->Selected].pos;
			else
				pos2 = multi_view->GetCursor3d();
			message = _("Wuerfel: dritter Punkt");
			pos2_chosen = true;
		}
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Wuefel: zweiter Punkt");
		pos_chosen = true;
	}
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
	if (pos_chosen){
		if (!pos2_chosen){
			vector pos2 = multi_view->GetCursor3d();
			vector dir0 = multi_view->GetDirection(multi_view->mouse_win);
			vector dir1 = multi_view->GetDirectionUp(multi_view->mouse_win);
			vector dir2 = multi_view->GetDirectionRight(multi_view->mouse_win);
			length[0] = dir1 * VecDotProduct(dir1, pos2 - pos);
			length[1] = dir2 * VecDotProduct(dir2, pos2 - pos);
			invert = (((length[0] ^ length[1]) * dir0) > 0);
		}else{
			vector pos3 = multi_view->GetCursor3d();
			vector n = length[0] ^ length[1];
			VecNormalize(n);
			length[2] = n * (n * (pos3 - pos));
		}
	}
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
	mode_model->SetMaterialCreation();
	if (pos_chosen){
		NixVBClear(VBTemp);
		if (!pos2_chosen){
			vector n = length[0] ^ length[1];
			VecNormalize(n);
			/// vertices
			vector a = pos;
			vector b = pos + length[0];
			vector c = pos + length[1];
			vector d = pos + length[0] + length[1];
			NixVBClear(VBTemp);
			NixVBAddTria(VBTemp, a, -n, 0, 0, c, -n, 0, 0, d, -n, 0, 0);
			NixVBAddTria(VBTemp, a, -n, 0, 0, d, -n, 0, 0, b, -n, 0, 0);
			NixVBAddTria(VBTemp, b,  n, 0, 0, d,  n, 0, 0, c,  n, 0, 0);
			NixVBAddTria(VBTemp, b,  n, 0, 0, c,  n, 0, 0, a,  n, 0, 0);
		}else{
			vector d[3];
			for (int i=0;i<3;i++)
				d[i] = length[i];
			if ((d[0] ^ d[1]) * d[2] < 0){
				d[0] = length[1];
				d[1] = length[0];
			}
			// 8 vertices
			vector _0 = pos;
			vector _1 = pos + d[0];
			vector _2 = pos        + d[1];
			vector _3 = pos + d[0] + d[1];
			vector _4 = pos               + d[2];
			vector _5 = pos + d[0]        + d[2];
			vector _6 = pos        + d[1] + d[2];
			vector _7 = pos + d[0] + d[1] + d[2];
			vector n0 = d[0];
			VecNormalize(n0);
			vector n1 = d[1];
			VecNormalize(n1);
			vector n2 = d[2];
			VecNormalize(n2);
			// front
			NixVBAddTria(VBTemp,_0,-n0,0,0,_2,-n0,0,0,_3,-n0,0,0);
			NixVBAddTria(VBTemp,_0,-n0,0,0,_3,-n0,0,0,_1,-n0,0,0);
			// top
			NixVBAddTria(VBTemp,_2, n1,0,0,_6, n1,0,0,_7, n1,0,0);
			NixVBAddTria(VBTemp,_2, n1,0,0,_7, n1,0,0,_3, n1,0,0);
			// bottom
			NixVBAddTria(VBTemp,_4,-n1,0,0,_0,-n1,0,0,_1,-n1,0,0);
			NixVBAddTria(VBTemp,_4,-n1,0,0,_1,-n1,0,0,_5,-n1,0,0);
			// left
			NixVBAddTria(VBTemp,_4,-n2,0,0,_6,-n2,0,0,_2,-n2,0,0);
			NixVBAddTria(VBTemp,_4,-n2,0,0,_2,-n2,0,0,_0,-n2,0,0);
			// right
			NixVBAddTria(VBTemp,_1, n2,0,0,_3, n2,0,0,_7, n2,0,0);
			NixVBAddTria(VBTemp,_1, n2,0,0,_7, n2,0,0,_5, n2,0,0);
			// back
			NixVBAddTria(VBTemp,_5, n0,0,0,_7, n0,0,0,_6, n0,0,0);
			NixVBAddTria(VBTemp,_5, n0,0,0,_6, n0,0,0,_4, n0,0,0);
		}
		NixDraw3D(-1, VBTemp, m_id);
	}
}



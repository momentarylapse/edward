/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "ModeModelMeshCreatePlane.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Surface/ActionModelAddPlane.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(Mode *_parent) :
	ModeCreation(_parent)
{
	name = "ModelMeshCreatePlane";
	data = (DataModel*)_parent->GetData();

	message = _("Ebene: erster Punkt");
	pos_chosen = false;
	length[0] = length[1] = v0;


	// Dialog
	dialog = HuiCreateResourceDialog("new_plane_dialog", ed);
	dialog->SetInt("np_num_x", HuiConfigReadInt("NewPlaneNumX", 4));
	dialog->SetInt("np_num_y",HuiConfigReadInt("NewPlaneNumY", 4));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}

ModeModelMeshCreatePlane::~ModeModelMeshCreatePlane()
{
	delete(dialog);
}


void ModeModelMeshCreatePlane::OnLeftButtonDown()
{
	if (pos_chosen){
		int nx = dialog->GetInt("np_num_x");
		int ny = dialog->GetInt("np_num_y");
		HuiConfigWriteInt("NewPlaneNumX", nx);
		HuiConfigWriteInt("NewPlaneNumY", ny);

		if (invert){
			pos += length[0];
			length[0] = -length[0];
		}

		data->Execute(new ActionModelAddPlane(data, pos, length[0], length[1], nx, ny));

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Ebene: zweiter Punkt");
		pos_chosen = true;
	}
}



void ModeModelMeshCreatePlane::OnDrawWin(int win, irect dest)
{
	if (pos_chosen){
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
		mode_model->SetMaterialCreation();
		NixDraw3D(-1, VBTemp, m_id);
	}
}



void ModeModelMeshCreatePlane::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d();
		vector dir0 = multi_view->GetDirection(multi_view->mouse_win);
		vector dir1 = multi_view->GetDirectionUp(multi_view->mouse_win);
		vector dir2 = multi_view->GetDirectionRight(multi_view->mouse_win);
		length[0] = dir1 * VecDotProduct(dir1, pos2 - pos);
		length[1] = dir2 * VecDotProduct(dir2, pos2 - pos);
		invert = (((length[0] ^ length[1]) * dir0) > 0);
	}
}



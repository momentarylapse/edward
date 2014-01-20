/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreatePlane.h"
#include "../../../../Data/Model/Geometry/GeometryPlane.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePlane", _parent)
{
	message = _("Ebene: erster Punkt");
	pos_chosen = false;
	length[0] = length[1] = v_0;
}

ModeModelMeshCreatePlane::~ModeModelMeshCreatePlane()
{
}

void ModeModelMeshCreatePlane::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_plane_dialog", ed);
	dialog->SetInt("np_num_x", HuiConfigReadInt("NewPlaneNumX", 4));
	dialog->SetInt("np_num_y",HuiConfigReadInt("NewPlaneNumY", 4));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate("");
}


void ModeModelMeshCreatePlane::OnEnd()
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

		GeometryPlane pl = GeometryPlane(pos, length[0], length[1], nx, ny);
		data->PasteGeometry(pl, mode_model_mesh->CurrentMaterial);
		data->SelectOnlySurface(&data->Surface.back());

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



void ModeModelMeshCreatePlane::OnDrawWin(MultiViewWindow *win)
{
	if (pos_chosen){
		vector n = length[0] ^ length[1];
		n.normalize();
		/// vertices
		vector a = pos;
		vector b = pos + length[0];
		vector c = pos + length[1];
		vector d = pos + length[0] + length[1];
		VBTemp->clear();
		VBTemp->addTria(a, -n, 0, 0, c, -n, 0, 0, d, -n, 0, 0);
		VBTemp->addTria(a, -n, 0, 0, d, -n, 0, 0, b, -n, 0, 0);
		VBTemp->addTria(b,  n, 0, 0, d,  n, 0, 0, c,  n, 0, 0);
		VBTemp->addTria(b,  n, 0, 0, c,  n, 0, 0, a,  n, 0, 0);
		mode_model->SetMaterialCreation();
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreatePlane::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d();
		vector dir0, dir1, dir2;
		multi_view->mouse_win->GetMovingFrame(dir0, dir1, dir2);
		length[0] = dir1 * VecDotProduct(dir1, pos2 - pos);
		length[1] = dir2 * VecDotProduct(dir2, pos2 - pos);
		invert = (((length[0] ^ length[1]) * dir0) > 0);
	}
}



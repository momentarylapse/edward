/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreateCube.h"
#include "../../../../Data/Model/Geometry/ModelGeometryCube.h"
#include "../../../../Data/Model/Geometry/ModelGeometryPlane.h"
#include "../../../../Edward.h"

ModeModelMeshCreateCube::ModeModelMeshCreateCube(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCube", _parent)
{
	message = _("W&urfel: Punkt 1 / 3");
	pos_chosen = false;
	pos2_chosen = false;
	for (int i=0;i<3;i++)
		length[i] = v_0;
	geo = NULL;
}

ModeModelMeshCreateCube::~ModeModelMeshCreateCube()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCube::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos2_chosen){
		int num_1 = dialog->GetInt("nc_x");
		int num_2 = dialog->GetInt("nc_y");
		int num_3 = dialog->GetInt("nc_z");
		HuiConfigWriteInt("NewCubeNumX", num_1);
		HuiConfigWriteInt("NewCubeNumY", num_2);
		HuiConfigWriteInt("NewCubeNumZ", num_3);

		geo = new ModelGeometryCube(pos, length[0], length[1], length[2], num_1, num_2, num_3);
	}else{
		geo = new ModelGeometryPlane(pos, length[0], length[1], 1, 1);
	}
}


void set_dpos3(vector *length, const vector &dpos)
{
	vector n = length[0] ^ length[1];
	n.normalize();
	length[2] = n * (n * dpos);
	float min_thick = 10 / ed->multi_view_3d->cam.zoom; // 10 px
	if (length[2].length() < min_thick)
		length[2] = n * min_thick;
}



void ModeModelMeshCreateCube::OnLeftButtonDown()
{
	if (pos_chosen){
		if (pos2_chosen){

			data->PasteGeometry(*geo, mode_model_mesh->CurrentMaterial);
			data->SelectOnlySurface(&data->Surface.back());

			Abort();
		}else{
			if (multi_view->Selected >= 0)
				pos2 = data->Vertex[multi_view->Selected].pos;
			else
				pos2 = multi_view->GetCursor3d();
			message = _("W&urfel: Punkt 3 / 3");
			pos2_chosen = true;
			set_dpos3(length, v_0);
			UpdateGeometry();
		}
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("W&urfel: Punkt 2 / 3");
		pos_chosen = true;
		UpdateGeometry();
	}
}

void ModeModelMeshCreateCube::OnMouseMove()
{
	if (pos_chosen){
		if (!pos2_chosen){
			vector pos2 = multi_view->GetCursor3d();
			vector dir0 = multi_view->mouse_win->GetDirectionRight();
			vector dir1 = multi_view->mouse_win->GetDirectionUp();
			length[0] = dir0 * VecDotProduct(dir0, pos2 - pos);
			length[1] = dir1 * VecDotProduct(dir1, pos2 - pos);
			UpdateGeometry();
		}else{
			set_dpos3(length, multi_view->GetCursor3d() - pos);
			UpdateGeometry();
		}
	}
}



void ModeModelMeshCreateCube::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_cube_dialog", ed);

	dialog->SetInt("nc_x", HuiConfigReadInt("NewCubeNumX", 1));
	dialog->SetInt("nc_y", HuiConfigReadInt("NewCubeNumY", 1));
	dialog->SetInt("nc_z", HuiConfigReadInt("NewCubeNumZ", 1));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}

void ModeModelMeshCreateCube::OnEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateCube::OnDrawWin(MultiViewWindow *win)
{
	mode_model->SetMaterialCreation();
	if (pos_chosen){
		geo->Preview(VBTemp);
		NixSetCull(pos2_chosen ? CullCCW : CullNone);
		NixDraw3D(VBTemp);
		NixSetCull(CullCCW);
	}
}



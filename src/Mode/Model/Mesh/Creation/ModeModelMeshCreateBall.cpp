/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/ModelGeometryBall.h"
#include "../../../../Data/Model/Geometry/ModelGeometrySphere.h"
#include "../../../../Edward.h"

ModeModelMeshCreateBall::ModeModelMeshCreateBall(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateBall", _parent)
{
	message = _("Kugelzentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateBall::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_ball_dialog", ed);

	dialog->SetInt("nb_x", HuiConfigReadInt("NewBallNumX", 8));
	dialog->SetInt("nb_y",HuiConfigReadInt("NewBallNumY", 16));
	dialog->SetInt("nb_complexity", HuiConfigReadInt("NewBallComplexity", 8));
	dialog->SetInt("nb_tab_control", HuiConfigReadBool("NewBallSphere", false) ? 1 : 0);
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreateBall::OnEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateBall::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		bool sphere = (dialog->GetInt("nb_tab_control") == 1);
		int nx = dialog->GetInt("nb_x");
		int ny = dialog->GetInt("nb_y");
		int complexity = dialog->GetInt("nb_complexity");
		HuiConfigWriteInt("NewBallNumX", nx);
		HuiConfigWriteInt("NewBallNumY", ny);
		HuiConfigWriteInt("NewBallComplexity", complexity);
		HuiConfigWriteBool("NewBallSphere", sphere);
		if (sphere)
			geo = new ModelGeometrySphere(pos, radius, complexity);
		else
			geo = new ModelGeometryBall(pos, radius, nx, ny);
	}
}



void ModeModelMeshCreateBall::OnLeftButtonDown()
{
	if (pos_chosen){
		data->PasteGeometry(*geo, mode_model_mesh->CurrentMaterial);
		data->SelectOnlySurface(&data->Surface.back());

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Kugel skalieren");
		pos_chosen = true;
		UpdateGeometry();
	}
}


void ModeModelMeshCreateBall::OnDrawWin(MultiViewWindow *win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		geo->Preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreateBall::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		radius = (pos2 - pos).length();
		UpdateGeometry();
	}
}



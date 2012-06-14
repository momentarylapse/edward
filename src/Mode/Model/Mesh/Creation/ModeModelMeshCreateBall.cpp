/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Surface/ActionModelAddBall.h"
#include "../../../../lib/x/x.h"

ModeModelMeshCreateBall::ModeModelMeshCreateBall(Mode *_parent) :
	ModeCreation(_parent)
{
	name = "ModelMeshCreateBall";
	data = (DataModel*)_parent->GetData();

	message = _("Kugelzentrum waehlen");

	pos_chosen = false;
	radius = 0;
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall()
{
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
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreateBall::OnEnd()
{
	delete(dialog);
}



void ModeModelMeshCreateBall::OnLeftButtonDown()
{
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
			data->Execute(new ActionModelAddBall(data, pos, radius, complexity, complexity, true));
		else
			data->Execute(new ActionModelAddBall(data, pos, radius, nx, ny, false));

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Kugel skalieren");
		pos_chosen = true;
	}
}


void ModeModelMeshCreateBall::OnDrawWin(int win, irect dest)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		FxDrawBall(pos, radius, 16,32);
	}
}



void ModeModelMeshCreateBall::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		radius = VecLength(pos2 - pos);
	}
}



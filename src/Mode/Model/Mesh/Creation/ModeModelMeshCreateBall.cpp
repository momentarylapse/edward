/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/ActionModelAddBall.h"
#include "../../../../lib/x/x.h"

ModeModelMeshCreateBall::ModeModelMeshCreateBall(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshCreateBall";
	parent = _parent;
	data = _data;
	multi_view = parent->multi_view;

	message = _("Kugelzentrum waehlen");

	pos_chosen = false;
	radius = 0;


	// Dialog
	dialog = HuiCreateResourceDialog("new_ball_dialog", ed->win);

	dialog->SetInt("nb_x", HuiConfigReadInt("NewBallNumX", 8));
	dialog->SetInt("nb_y",HuiConfigReadInt("NewBallNumY", 16));
	dialog->SetInt("nb_complexity", HuiConfigReadInt("NewBallComplexity", 8));
	dialog->SetInt("nb_tab_control", HuiConfigReadBool("NewBallSphere", false) ? 1 : 0);
	dialog->SetPositionSpecial(ed->win, HuiRight | HuiTop);
	dialog->Update();

	ed->win->Activate();
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall()
{
	delete(dialog);
}

void ModeModelMeshCreateBall::OnMiddleButtonDown()
{
}



void ModeModelMeshCreateBall::OnKeyDown()
{
}



void ModeModelMeshCreateBall::OnLeftButtonUp()
{
}



void ModeModelMeshCreateBall::OnRightButtonUp()
{
}



void ModeModelMeshCreateBall::OnMiddleButtonUp()
{
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
			data->Execute(new ActionModelAddBall(pos, radius, complexity, complexity, true, data->Vertex.num));
		else
			data->Execute(new ActionModelAddBall(pos, radius, nx, ny, false, data->Vertex.num));

		ed->SetCreationMode(NULL);
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Kugel skalieren");
		pos_chosen = true;
	}
}



void ModeModelMeshCreateBall::PostDraw()
{
}



void ModeModelMeshCreateBall::Start()
{
}



void ModeModelMeshCreateBall::OnKeyUp()
{
}



void ModeModelMeshCreateBall::OnCommand(const string & id)
{
}


void ModeModelMeshCreateBall::PostDrawWin(int win, irect dest)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		FxDrawBall(pos, radius, 16,32);
	}
}



void ModeModelMeshCreateBall::End()
{
}



void ModeModelMeshCreateBall::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d();
		radius = VecLength(pos2 - pos);
	}
}



void ModeModelMeshCreateBall::OnRightButtonDown()
{
}



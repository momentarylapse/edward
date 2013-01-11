/*
 * ModeModelMeshCreatePlatonic.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreatePlatonic.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../lib/x/x.h"

ModeModelMeshCreatePlatonic::ModeModelMeshCreatePlatonic(Mode *_parent, int _type) :
	ModeCreation("ModelMeshCreatePlatonic", _parent)
{
	data = (DataModel*)_parent->GetData();
	type = _type;

	message = _("Zentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
}

ModeModelMeshCreatePlatonic::~ModeModelMeshCreatePlatonic()
{
}



void ModeModelMeshCreatePlatonic::OnLeftButtonDown()
{
	if (pos_chosen){
		ModelSurface *s = data->AddPlatonic(pos, radius, type);
		data->SelectOnlySurface(s);

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("skalieren");
		pos_chosen = true;
	}
}


void ModeModelMeshCreatePlatonic::OnDrawWin(int win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		FxDrawBall(pos, radius, 16,32);
	}
}



void ModeModelMeshCreatePlatonic::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		radius = (pos2 - pos).length();
	}
}



/*
 * ModeModelMeshCreateDodecahedron.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateDodecahedron.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../lib/x/x.h"

ModeModelMeshCreateDodecahedron::ModeModelMeshCreateDodecahedron(Mode *_parent) :
	ModeCreation("ModelMeshCreateDodecahedron", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Dodekaeder-Zentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
}

ModeModelMeshCreateDodecahedron::~ModeModelMeshCreateDodecahedron()
{
}



void ModeModelMeshCreateDodecahedron::OnLeftButtonDown()
{
	if (pos_chosen){
		ModelSurface *s = data->AddDodecahedron(pos, radius);
		data->SelectOnlySurface(s);

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Dodekaeder skalieren");
		pos_chosen = true;
	}
}


void ModeModelMeshCreateDodecahedron::OnDrawWin(int win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		FxDrawBall(pos, radius, 16,32);
	}
}



void ModeModelMeshCreateDodecahedron::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		radius = (pos2 - pos).length();
	}
}



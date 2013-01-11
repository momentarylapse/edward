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
#include "../../../../Data/Model/Geometry/ModelGeometryPlatonic.h"
#include "../../../../Data/Model/Geometry/ModelGeometryTeapot.h"

//extern int FxVB;

ModeModelMeshCreatePlatonic::ModeModelMeshCreatePlatonic(Mode *_parent, int _type) :
	ModeCreation("ModelMeshCreatePlatonic", _parent)
{
	data = (DataModel*)_parent->GetData();
	type = _type;

	message = _("Zentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
	geo = NULL;
}

ModeModelMeshCreatePlatonic::~ModeModelMeshCreatePlatonic()
{
	if (geo)
		delete(geo);
}


void ModeModelMeshCreatePlatonic::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		if (type == 306)
			geo = new ModelGeometryTeapot(pos, radius, 4);
		else
			geo = new ModelGeometryPlatonic(pos, radius, type);
	}
}


void ModeModelMeshCreatePlatonic::OnLeftButtonDown()
{
	if (pos_chosen){
		data->PasteGeometry(*geo);

		Abort();
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("skalieren");
		pos_chosen = true;
		UpdateGeometry();
	}
}


void ModeModelMeshCreatePlatonic::OnDrawWin(int win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();

		geo->Preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreatePlatonic::OnMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		radius = (pos2 - pos).length();
		UpdateGeometry();
	}
}



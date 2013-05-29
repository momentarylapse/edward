/*
 * ModeModelMeshCreatePlatonic.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreatePlatonic.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Data/Model/Geometry/ModelGeometryPlatonic.h"
#include "../../../../Data/Model/Geometry/ModelGeometryTeapot.h"

//extern int FxVB;

ModeModelMeshCreatePlatonic::ModeModelMeshCreatePlatonic(ModeBase *_parent, int _type) :
	ModeCreation<DataModel>("ModelMeshCreatePlatonic", _parent)
{
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

void ModeModelMeshCreatePlatonic::OnStart()
{
	if (type != 306)
		return;
	// Dialog
	dialog = HuiCreateResourceDialog("new_teapot_dialog", ed);
	dialog->SetInt("ntp_samples", HuiConfigReadInt("NewTeapotSamples", 4));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreatePlatonic::OnEnd()
{
	if (dialog)
		delete(dialog);
}


void ModeModelMeshCreatePlatonic::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		if (type == 306){
			int samples = dialog->GetInt("ntp_samples");
			HuiConfigWriteInt("NewTeapotSamples", samples);
			geo = new ModelGeometryTeapot(pos, radius, samples);
		}else{
			geo = new ModelGeometryPlatonic(pos, radius, type);
		}
	}
}


void ModeModelMeshCreatePlatonic::OnLeftButtonDown()
{
	if (pos_chosen){
		data->PasteGeometry(*geo, mode_model_mesh->CurrentMaterial);

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


void ModeModelMeshCreatePlatonic::OnDrawWin(MultiViewWindow *win)
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



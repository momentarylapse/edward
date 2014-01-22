/*
 * ModeModelMeshCreateTorus.cpp
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTorus.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryTorus.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiViewWindow.h"

#define RADIUS_FACTOR	0.5f

ModeModelMeshCreateTorus::ModeModelMeshCreateTorus(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateTorus", _parent)
{
	message = _("Toruszentrum w&ahlen");

	pos_chosen = false;
	rad_chosen = false;
	radius1 = 0;
	radius2 = 0;
	axis = e_z;
	geo = NULL;
}

ModeModelMeshCreateTorus::~ModeModelMeshCreateTorus()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateTorus::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_torus_dialog", ed);

	dialog->SetInt("nt_rings", HuiConfigReadInt("NewTorusNumX", 32));
	dialog->SetInt("nt_edges", HuiConfigReadInt("NewTorusNumY", 16));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate("");
}


void ModeModelMeshCreateTorus::OnEnd()
{
	delete(dialog);
}


void ModeModelMeshCreateTorus::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		int nx = dialog->GetInt("nt_rings");
		int ny = dialog->GetInt("nt_edges");
		HuiConfigWriteInt("NewTorusNumX", nx);
		HuiConfigWriteInt("NewTorusNumY", ny);
		geo = new GeometryTorus(pos, axis, radius1, radius2, nx, ny);
	}
}


void ModeModelMeshCreateTorus::OnLeftButtonDown()
{
	if (pos_chosen){
		if (rad_chosen){

			data->PasteGeometry(*geo, mode_model_mesh->CurrentMaterial);
			data->SelectOnlySurface(&data->Surface.back());

			Abort();
		}else{
			message = _("Torus innen skalieren");
			rad_chosen = true;
		}
	}else{
		if (multi_view->selection.index >= 0)
			pos = data->Vertex[multi_view->selection.index].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Torus au&sen skalieren");
		pos_chosen = true;
		UpdateGeometry();
	}
}


void ModeModelMeshCreateTorus::OnDrawWin(MultiViewWindow *win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		geo->Preview(VBTemp);
		NixDraw3D(VBTemp);
		NixEnableLighting(false);
		ed->DrawStr(100, 100, format("%.3f / %.3f", radius1, radius2));
	}
}



void ModeModelMeshCreateTorus::OnMouseMove()
{
	axis = multi_view->mouse_win->GetDirection();
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		if (rad_chosen){
			radius2 = (pos2 - pos).length() * RADIUS_FACTOR;
		}else{
			radius1 = (pos2 - pos).length();
			radius2 = radius1 * RADIUS_FACTOR;
		}
		UpdateGeometry();
	}
}



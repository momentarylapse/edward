/*
 * ModeModelMeshCreateTorus.cpp
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTorus.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../lib/x/x.h"
#include "../../../../Action/Model/Mesh/Shape/ActionModelAddTorus.h"

// ModeMaterial
void CreateTorus(int buffer, const vector &pos, const vector dir, float radius1, float radius2, int nx, int ny);

#define RADIUS_FACTOR	0.7f

ModeModelMeshCreateTorus::ModeModelMeshCreateTorus(Mode *_parent) :
	ModeCreation("ModelMeshCreateTorus", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Toruszentrum w&ahlen");

	pos_chosen = false;
	rad_chosen = false;
	radius1 = 0;
	radius2 = 0;
	axis = e_z;
}

ModeModelMeshCreateTorus::~ModeModelMeshCreateTorus()
{
}

void ModeModelMeshCreateTorus::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_torus_dialog", ed);

	dialog->SetInt("nc_x", HuiConfigReadInt("NewTorusNumX", 32));
	dialog->SetInt("nc_y", HuiConfigReadInt("NewTorusNumY", 16));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Update();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate();
}


void ModeModelMeshCreateTorus::OnEnd()
{
	delete(dialog);
}



void ModeModelMeshCreateTorus::OnLeftButtonDown()
{
	if (pos_chosen){
		if (rad_chosen){
			int nx = dialog->GetInt("nc_x");
			int ny = dialog->GetInt("nc_y");
			HuiConfigWriteInt("NewTorusNumX", nx);
			HuiConfigWriteInt("NewTorusNumY", ny);

			ModelSurface *s = data->AddTorus(pos, axis, radius1, radius2, nx, ny);
			data->SelectOnlySurface(s);

			Abort();
		}else{
			message = _("Torus innen skalieren");
			rad_chosen = true;
		}
	}else{
		if (multi_view->Selected >= 0)
			pos = data->Vertex[multi_view->Selected].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Torus au&sen skalieren");
		pos_chosen = true;
	}
}


void ModeModelMeshCreateTorus::OnDrawWin(int win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		NixVBClear(VBTemp);
		CreateTorus(VBTemp, pos, axis, radius1, radius2, 32, 16);
		NixDraw3D(VBTemp);
		NixEnableLighting(false);
		ed->DrawStr(100, 100, format("%.3f / %.3f", radius1, radius2));
	}
}



void ModeModelMeshCreateTorus::OnMouseMove()
{
	axis = multi_view->GetDirection(multi_view->mouse_win);
	if (pos_chosen){
		vector pos2 = multi_view->GetCursor3d(pos);
		if (rad_chosen){
			radius2 = (pos2 - pos).length() * RADIUS_FACTOR;
		}else{
			radius1 = (pos2 - pos).length();
			radius2 = radius1 * RADIUS_FACTOR;
		}
	}
}



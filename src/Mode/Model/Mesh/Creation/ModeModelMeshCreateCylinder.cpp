/*
 * ModeModelMeshCreateCylinder.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinder.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshCreateCylinder::ModeModelMeshCreateCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCylinder", _parent)
{
	message = _("Zylinder: Startpunkt");

	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateCylinder::~ModeModelMeshCreateCylinder()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCylinder::OnStart()
{
	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->SetInt("ncy_rings", HuiConfigReadInt("NewCylinderRings", 4));
	dialog->SetInt("ncy_edges", HuiConfigReadInt("NewCylinderEdges", 8));
	dialog->SetPositionSpecial(ed, HuiRight | HuiTop);
	dialog->Show();
	dialog->Event("hui:close", &HuiFuncIgnore);

	ed->Activate("");
}


void ModeModelMeshCreateCylinder::OnEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateCylinder::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos.num == 2){
		int rings = dialog->GetInt("ncy_rings");
		int edges = dialog->GetInt("ncy_edges");
		HuiConfigWriteInt("NewCylinderRings", rings);
		HuiConfigWriteInt("NewCylinderEdges", edges);

		Array<float> r = radius;
		r += radius;
		geo = new GeometryCylinder(pos, r, rings, edges, false);
	}
}

void ModeModelMeshCreateCylinder::OnMouseMove()
{
	if (pos.num == 2){
		vector p = multi_view->GetCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->cam.zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
		UpdateGeometry();
	}
}



void ModeModelMeshCreateCylinder::OnLeftButtonDown()
{
	if (pos.num == 2){


		data->PasteGeometry(*geo, mode_model_mesh->CurrentMaterial);
		data->SelectOnlySurface(&data->Surface.back());

		Abort();
	}else{
		if (multi_view->selection.index >= 0)
			pos.add(data->Vertex[multi_view->selection.index].pos);
		else
			pos.add(multi_view->GetCursor3d());

		if (pos.num > 1){
			//OnMouseMove();
			message = _("Zylinder: Radius");
			UpdateGeometry();
			//ed->ForceRedraw();
		}else{
			message = _("Zylinder: Endpunkt");
		}
	}
}

void ModeModelMeshCreateCylinder::OnDrawWin(MultiView::Window *win)
{
	if (pos.num > 0){
		NixEnableLighting(false);
		NixSetColor(Green);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = win->Project(pos[i]);
			NixDrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}
		if (pos.num == 2)
			NixDrawLine3D(pos[0], pos[1]);
		else
			NixDrawLine3D(pos[0], multi_view->GetCursor3d());
		NixSetColor(White);
	}
	if (pos.num == 2){
		NixEnableLighting(true);
		mode_model->SetMaterialCreation();
		geo->Preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



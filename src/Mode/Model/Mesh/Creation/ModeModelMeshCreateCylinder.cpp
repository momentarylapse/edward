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

void ModeModelMeshCreateCylinder::onStart()
{
	dialog = HuiCreateResourceDialog("new_cylinder_dialog",ed);

	dialog->setInt("ncy_rings", HuiConfig.getInt("NewCylinderRings", 4));
	dialog->setInt("ncy_edges", HuiConfig.getInt("NewCylinderEdges", 8));
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);

	ed->activate("");
}


void ModeModelMeshCreateCylinder::onEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateCylinder::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos.num == 2){
		int rings = dialog->getInt("ncy_rings");
		int edges = dialog->getInt("ncy_edges");
		HuiConfig.setInt("NewCylinderRings", rings);
		HuiConfig.setInt("NewCylinderEdges", edges);

		Array<float> r = radius;
		r += radius;
		geo = new GeometryCylinder(pos, r, rings, edges, false);
	}
}

void ModeModelMeshCreateCylinder::onMouseMove()
{
	if (pos.num == 2){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		float min_rad = 10 / multi_view->cam.zoom; // 10 px
		if (radius < min_rad)
			radius = min_rad;
		UpdateGeometry();
	}
}



void ModeModelMeshCreateCylinder::onLeftButtonDown()
{
	if (pos.num == 2){


		data->pasteGeometry(*geo, mode_model_mesh->current_material);
		data->selectOnlySurface(&data->surface.back());

		abort();
	}else{
		if (multi_view->hover.index >= 0)
			pos.add(data->vertex[multi_view->hover.index].pos);
		else
			pos.add(multi_view->getCursor3d());

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

void ModeModelMeshCreateCylinder::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos.num > 0){
		NixEnableLighting(false);
		NixSetColor(Green);
		// control polygon
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			NixDrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}
		if (pos.num == 2)
			NixDrawLine3D(pos[0], pos[1]);
		else
			NixDrawLine3D(pos[0], multi_view->getCursor3d());
		NixSetColor(White);
	}
	if (pos.num == 2){
		NixEnableLighting(true);
		mode_model->setMaterialCreation();
		geo->preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



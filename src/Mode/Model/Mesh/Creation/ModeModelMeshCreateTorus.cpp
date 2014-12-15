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
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

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

void ModeModelMeshCreateTorus::onStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("new_torus_dialog", ed);

	dialog->setInt("nt_rings", HuiConfig.getInt("NewTorusNumX", 32));
	dialog->setInt("nt_edges", HuiConfig.getInt("NewTorusNumY", 16));
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);

	ed->activate("");
}


void ModeModelMeshCreateTorus::onEnd()
{
	delete(dialog);
}


void ModeModelMeshCreateTorus::UpdateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		int nx = dialog->getInt("nt_rings");
		int ny = dialog->getInt("nt_edges");
		HuiConfig.setInt("NewTorusNumX", nx);
		HuiConfig.setInt("NewTorusNumY", ny);
		geo = new GeometryTorus(pos, axis, radius1, radius2, nx, ny);
	}
}


void ModeModelMeshCreateTorus::onLeftButtonDown()
{
	if (pos_chosen){
		if (rad_chosen){

			data->PasteGeometry(*geo, mode_model_mesh->current_material);
			data->SelectOnlySurface(&data->surface.back());

			abort();
		}else{
			message = _("Torus innen skalieren");
			rad_chosen = true;
		}
	}else{
		if (multi_view->hover.index >= 0)
			pos = data->vertex[multi_view->hover.index].pos;
		else
			pos = multi_view->GetCursor3d();
		message = _("Torus au&sen skalieren");
		pos_chosen = true;
		UpdateGeometry();
	}
}


void ModeModelMeshCreateTorus::onDrawWin(MultiView::Window *win)
{
	if (pos_chosen){
		mode_model->SetMaterialCreation();
		geo->preview(VBTemp);
		NixDraw3D(VBTemp);
		NixEnableLighting(false);
		ed->drawStr(100, 100, format("%.3f / %.3f", radius1, radius2));
	}
}



void ModeModelMeshCreateTorus::onMouseMove()
{
	axis = multi_view->mouse_win->getDirection();
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



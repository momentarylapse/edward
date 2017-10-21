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
	dialog = hui::CreateResourceDialog("new_torus_dialog", ed);

	dialog->setInt("nt_rings", hui::Config.getInt("NewTorusNumX", 32));
	dialog->setInt("nt_edges", hui::Config.getInt("NewTorusNumY", 16));
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateTorus::onClose, this));

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateTorus::onEnd()
{
	delete(dialog);
}

void ModeModelMeshCreateTorus::onClose()
{
	abort();
}

void ModeModelMeshCreateTorus::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		int nx = dialog->getInt("nt_rings");
		int ny = dialog->getInt("nt_edges");
		hui::Config.setInt("NewTorusNumX", nx);
		hui::Config.setInt("NewTorusNumY", ny);
		geo = new GeometryTorus(pos, axis, radius1, radius2, nx, ny);
	}
}


void ModeModelMeshCreateTorus::onLeftButtonUp()
{
	if (pos_chosen){
		if (rad_chosen){

			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());

			abort();
		}else{
			message = _("Torus innen skalieren");
			rad_chosen = true;
		}
	}else{
		pos = multi_view->getCursor3d();
		message = _("Torus au&sen skalieren");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateTorus::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		mode_model->setMaterialCreation();
		geo->preview(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
		nix::EnableLighting(false);
		ed->drawStr(100, 100, format("%.3f / %.3f", radius1, radius2));
	}
}



void ModeModelMeshCreateTorus::onMouseMove()
{
	axis = multi_view->mouse_win->getDirection();
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d(pos);
		if (rad_chosen){
			radius2 = (pos2 - pos).length() * RADIUS_FACTOR;
		}else{
			radius1 = (pos2 - pos).length();
			radius2 = radius1 * RADIUS_FACTOR;
		}
		updateGeometry();
	}
}



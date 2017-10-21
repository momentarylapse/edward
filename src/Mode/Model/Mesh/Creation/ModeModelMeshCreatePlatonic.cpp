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
#include "../../../../Data/Model/Geometry/GeometryPlatonic.h"
#include "../../../../Data/Model/Geometry/GeometryTeapot.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

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

void ModeModelMeshCreatePlatonic::onStart()
{
	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	if (type != 306)
		return;
	// Dialog
	dialog = hui::CreateResourceDialog("new_teapot_dialog", ed);
	dialog->setInt("ntp_samples", hui::Config.getInt("NewTeapotSamples", 4));
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreatePlatonic::onClose, this));

	ed->activate("");
}


void ModeModelMeshCreatePlatonic::onEnd()
{
	if (dialog)
		delete(dialog);
}

void ModeModelMeshCreatePlatonic::onClose()
{
	abort();
}

void ModeModelMeshCreatePlatonic::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		if (type == 306){
			int samples = dialog->getInt("ntp_samples");
			hui::Config.setInt("NewTeapotSamples", samples);
			geo = new GeometryTeapot(pos, radius, samples);
		}else{
			geo = new GeometryPlatonic(pos, radius, type);
		}
	}
}


void ModeModelMeshCreatePlatonic::onLeftButtonUp()
{
	if (pos_chosen){
		data->pasteGeometry(*geo, mode_model_mesh->current_material);

		abort();
	}else{
		pos = multi_view->getCursor3d();
		message = _("skalieren");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreatePlatonic::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		mode_model->setMaterialCreation();

		geo->preview(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}
}



void ModeModelMeshCreatePlatonic::onMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d(pos);
		radius = (pos2 - pos).length();
		updateGeometry();
	}
}



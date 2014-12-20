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
	if (type != 306)
		return;
	// Dialog
	dialog = HuiCreateResourceDialog("new_teapot_dialog", ed);
	dialog->setInt("ntp_samples", HuiConfig.getInt("NewTeapotSamples", 4));
	dialog->setPositionSpecial(ed, HuiRight | HuiTop);
	dialog->show();
	dialog->eventS("hui:close", &HuiFuncIgnore);

	ed->activate("");
}


void ModeModelMeshCreatePlatonic::onEnd()
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
			int samples = dialog->getInt("ntp_samples");
			HuiConfig.setInt("NewTeapotSamples", samples);
			geo = new GeometryTeapot(pos, radius, samples);
		}else{
			geo = new GeometryPlatonic(pos, radius, type);
		}
	}
}


void ModeModelMeshCreatePlatonic::onLeftButtonDown()
{
	if (pos_chosen){
		data->PasteGeometry(*geo, mode_model_mesh->current_material);

		abort();
	}else{
		if (multi_view->hover.index >= 0)
			pos = data->vertex[multi_view->hover.index].pos;
		else
			pos = multi_view->getCursor3d();
		message = _("skalieren");
		pos_chosen = true;
		UpdateGeometry();
	}
}


void ModeModelMeshCreatePlatonic::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	if (pos_chosen){
		mode_model->setMaterialCreation();

		geo->preview(VBTemp);
		NixDraw3D(VBTemp);
	}
}



void ModeModelMeshCreatePlatonic::onMouseMove()
{
	if (pos_chosen){
		vector pos2 = multi_view->getCursor3d(pos);
		radius = (pos2 - pos).length();
		UpdateGeometry();
	}
}



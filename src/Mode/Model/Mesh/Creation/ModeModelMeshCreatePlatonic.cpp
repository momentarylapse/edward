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
#include "../../../../lib/nix/nix.h"

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

void ModeModelMeshCreatePlatonic::on_start()
{
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	if (type != 306)
		return;
	// Dialog
	dialog = hui::CreateResourceDialog("new_teapot_dialog", ed);
	dialog->set_int("ntp_samples", hui::Config.get_int("NewTeapotSamples", 4));
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreatePlatonic::onClose, this));

	ed->activate("");
}


void ModeModelMeshCreatePlatonic::on_end()
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
			int samples = dialog->get_int("ntp_samples");
			hui::Config.set_int("NewTeapotSamples", samples);
			geo = new GeometryTeapot(pos, radius, samples);
		}else{
			geo = new GeometryPlatonic(pos, radius, type);
		}
	}
}


void ModeModelMeshCreatePlatonic::on_left_button_up()
{
	if (pos_chosen){
		data->pasteGeometry(*geo, mode_model_mesh->current_material);

		abort();
	}else{
		pos = multi_view->get_cursor();
		message = _("skalieren");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreatePlatonic::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	if (pos_chosen){
		mode_model->set_material_creation();

		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}
}



void ModeModelMeshCreatePlatonic::on_mouse_move()
{
	if (pos_chosen){
		vector pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		updateGeometry();
	}
}



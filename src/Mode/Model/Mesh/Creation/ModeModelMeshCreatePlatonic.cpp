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
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"


void draw_helper_line(MultiView::Window *win, const vec3 &a, const vec3 &b);

ModeModelMeshCreatePlatonic::ModeModelMeshCreatePlatonic(ModeBase *_parent, int _type) :
	ModeCreation<DataModel>("ModelMeshCreatePlatonic", _parent)
{
	type = _type;

	message = _("Select center");

	pos_chosen = false;
	radius = 0;
	geo = nullptr;
}

ModeModelMeshCreatePlatonic::~ModeModelMeshCreatePlatonic() {
	if (geo)
		delete geo;
}

void ModeModelMeshCreatePlatonic::on_start()
{
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	if (type == 306) {
		// teapot
		dialog = new hui::Panel();
		dialog->from_resource("new_teapot_dialog");
		dialog->set_int("ntp_samples", hui::config.get_int("NewTeapotSamples", 4));
		ed->set_side_panel(dialog);
	}
}


void ModeModelMeshCreatePlatonic::on_end() {
	if (dialog)
		ed->set_side_panel(nullptr);
}

void ModeModelMeshCreatePlatonic::updateGeometry() {
	if (geo)
		delete geo;
	if (pos_chosen) {
		if (type == 306) {
			int samples = dialog->get_int("ntp_samples");
			hui::config.set_int("NewTeapotSamples", samples);
			geo = new GeometryTeapot(pos, radius, samples);
		} else {
			geo = new GeometryPlatonic(pos, radius, type);
		}
	}
}


void ModeModelMeshCreatePlatonic::on_left_button_up() {
	if (pos_chosen) {
		data->pasteGeometry(*geo, mode_model_mesh->current_material);

		abort();
	} else {
		pos = multi_view->get_cursor();
		message = _("scale");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreatePlatonic::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos_chosen) {
		set_material_creation();

		geo->build(nix::vb_temp);
		nix::draw_triangles(nix::vb_temp);

		if (win == multi_view->mouse_win)
			draw_helper_line(win, pos, multi_view->get_cursor());
	}
}



void ModeModelMeshCreatePlatonic::on_mouse_move() {
	if (pos_chosen) {
		vec3 pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		updateGeometry();
	}
}



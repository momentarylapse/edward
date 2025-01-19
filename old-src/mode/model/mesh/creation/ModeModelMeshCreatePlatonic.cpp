/*
 * ModeModelMeshCreatePlatonic.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreatePlatonic.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../EdwardWindow.h"
#include "../../../../Session.h"
#include "../../../../data/model/geometry/GeometryPlatonic.h"
#include "../../../../data/model/geometry/GeometryTeapot.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/hui/config.h"


ModeModelMeshCreatePlatonic::ModeModelMeshCreatePlatonic(ModeModelMesh *_parent, int _type) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreatePlatonic", _parent)
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
		session->win->set_side_panel(dialog);
	}
}


void ModeModelMeshCreatePlatonic::on_end() {
	if (dialog)
		session->win->set_side_panel(nullptr);
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
		data->pasteGeometry(*geo, parent->current_material);

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
		win->drawing_helper->set_material_creation();
#if HAS_LIB_GL
		geo->build(win->ctx->vb_temp);
		nix::draw_triangles(win->ctx->vb_temp);
#endif

		if (win == multi_view->mouse_win)
			win->drawing_helper->draw_helper_line(win, pos, multi_view->get_cursor());
	}
}



void ModeModelMeshCreatePlatonic::on_mouse_move() {
	if (pos_chosen) {
		vec3 pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		updateGeometry();
	}
}



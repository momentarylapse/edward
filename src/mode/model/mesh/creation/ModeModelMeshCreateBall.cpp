/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/geometry/GeometryBall.h"
#include "../../../../data/model/geometry/GeometrySphere.h"
#include "../../../../action/model/mesh/physical/ActionModelAddBall.h"
#include "../../../../Session.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"



ModeModelMeshCreateBall::ModeModelMeshCreateBall(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreateBall", _parent)
{
	message = _("Select center of sphere");

	pos_chosen = false;
	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall() {
	if (geo)
		delete geo;
}

void ModeModelMeshCreateBall::onTypeBall() {
	dialog->enable("x", true);
	dialog->enable("y", true);
	dialog->enable("complexity", false);
}

void ModeModelMeshCreateBall::onTypeSphere() {
	dialog->enable("x", false);
	dialog->enable("y", false);
	dialog->enable("complexity", true);
}

void ModeModelMeshCreateBall::on_start() {
	dialog = new hui::Panel();
	dialog->from_resource("new_ball_dialog");

	dialog->set_int("x", hui::config.get_int("NewBallNumX", 8));
	dialog->set_int("y",hui::config.get_int("NewBallNumY", 16));
	dialog->set_int("complexity", hui::config.get_int("NewBallComplexity", 8));
	bool sphere = hui::config.get_bool("NewBallSphere", false);
	dialog->check("type:ball", !sphere);
	dialog->check("type:sphere", sphere);
	dialog->enable("x", !sphere);
	dialog->enable("y", !sphere);
	dialog->enable("complexity", sphere);
	dialog->event("type:ball", [=]{ onTypeBall(); });
	dialog->event("type:sphere", [=]{ onTypeSphere(); });
	session->win->set_side_panel(dialog);

	bool physical = (parent->current_skin == MESH_PHYSICAL);
	if (physical)
		dialog->enable("*", false);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}


void ModeModelMeshCreateBall::on_end() {
	session->win->set_side_panel(nullptr);
}

void ModeModelMeshCreateBall::updateGeometry() {
	if (geo)
		delete geo;
	if (pos_chosen) {
		bool ball = dialog->is_checked("type:ball");
		bool sphere = dialog->is_checked("type:sphere");
		int nx = dialog->get_int("x");
		int ny = dialog->get_int("y");
		int complexity = dialog->get_int("complexity");
		hui::config.set_int("NewBallNumX", nx);
		hui::config.set_int("NewBallNumY", ny);
		hui::config.set_int("NewBallComplexity", complexity);
		hui::config.set_bool("NewBallSphere", sphere);
		if (ball)
			geo = new GeometryBall(pos, radius, nx, ny);
		else if (sphere)
			geo = new GeometrySphere(pos, radius, complexity);
		else // physical
			geo = new GeometrySphere(pos, radius, 6);
	}
}



void ModeModelMeshCreateBall::on_left_button_up() {
	if (pos_chosen) {
		if (parent->current_skin == MESH_PHYSICAL) {
			ModelVertex v;
			v.pos = pos;
			ModelBall b;
			b.index = data->phys_mesh->vertex.num;
			b.radius = radius;
			data->phys_mesh->vertex.add(v);
			data->execute(new ActionModelAddBall(b));

		} else {
			data->pasteGeometry(*geo, ((ModeModelMesh*)parent)->current_material);
		}

		abort();
	} else {
		pos = multi_view->get_cursor();
		message = _("Sphere radius: ");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateBall::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos_chosen) {
		win->drawing_helper->set_material_creation();
		geo->build(win->gl->vb_temp);
		nix::draw_triangles(win->gl->vb_temp);

		if (win == multi_view->mouse_win)
			win->drawing_helper->draw_helper_line(win, pos, multi_view->get_cursor());
	}
}


void ModeModelMeshCreateBall::on_mouse_move() {
	if (pos_chosen) {
		vec3 pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		if (multi_view->snap_to_grid)
			radius = multi_view->snap_f(radius);
		updateGeometry();
		message = _("Sphere radius: ") + multi_view->format_length(radius);
	}
}



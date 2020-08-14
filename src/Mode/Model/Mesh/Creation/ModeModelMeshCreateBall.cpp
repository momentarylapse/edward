/*
 * ModeModelMeshCreateBall.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateBall.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryBall.h"
#include "../../../../Data/Model/Geometry/GeometrySphere.h"
#include "../../../../Action/Model/Mesh/Physical/ActionModelAddBall.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"



ModeModelMeshCreateBall::ModeModelMeshCreateBall(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateBall", _parent)
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

	dialog->set_int("x", hui::Config.get_int("NewBallNumX", 8));
	dialog->set_int("y",hui::Config.get_int("NewBallNumY", 16));
	dialog->set_int("complexity", hui::Config.get_int("NewBallComplexity", 8));
	bool sphere = hui::Config.get_bool("NewBallSphere", false);
	dialog->check("type:ball", !sphere);
	dialog->check("type:sphere", sphere);
	dialog->enable("x", !sphere);
	dialog->enable("y", !sphere);
	dialog->enable("complexity", sphere);
	dialog->event("type:ball", [=]{ onTypeBall(); });
	dialog->event("type:sphere", [=]{ onTypeSphere(); });
	ed->set_side_panel(dialog);

	bool physical = (mode_model_mesh->current_skin == MESH_PHYSICAL);
	if (physical)
		dialog->enable("*", false);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}


void ModeModelMeshCreateBall::on_end() {
	ed->set_side_panel(nullptr);
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
		hui::Config.set_int("NewBallNumX", nx);
		hui::Config.set_int("NewBallNumY", ny);
		hui::Config.set_int("NewBallComplexity", complexity);
		hui::Config.set_bool("NewBallSphere", sphere);
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
		if (mode_model_mesh->current_skin == MESH_PHYSICAL) {
			ModelVertex v;
			v.pos = pos;
			ModelBall b;
			b.index = data->phys_mesh->vertex.num;
			b.radius = radius;
			data->phys_mesh->vertex.add(v);
			data->execute(new ActionModelAddBall(b));

		} else {
			data->pasteGeometry(*geo, mode_model_mesh->current_material);
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
		mode_model->set_material_creation();
		geo->build(nix::vb_temp);
		nix::DrawTriangles(nix::vb_temp);

		if (win == multi_view->mouse_win)
			draw_helper_line(win, pos, multi_view->get_cursor());
	}
}


void ModeModelMeshCreateBall::on_mouse_move() {
	if (pos_chosen) {
		vector pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		if (multi_view->snap_to_grid)
			radius = multi_view->snap_f(radius);
		updateGeometry();
		message = _("Sphere radius: ") + multi_view->format_length(radius);
	}
}



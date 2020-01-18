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
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"



ModeModelMeshCreateBall::ModeModelMeshCreateBall(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateBall", _parent)
{
	message = _("Kugelzentrum w&ahlen");

	pos_chosen = false;
	radius = 0;
	geo = NULL;
}

ModeModelMeshCreateBall::~ModeModelMeshCreateBall()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateBall::onTypeBall()
{
	dialog->enable("x", true);
	dialog->enable("y", true);
	dialog->enable("complexity", false);
}

void ModeModelMeshCreateBall::onTypeSphere()
{
	dialog->enable("x", false);
	dialog->enable("y", false);
	dialog->enable("complexity", true);
}

void ModeModelMeshCreateBall::onTypePhysical()
{
	dialog->enable("x", false);
	dialog->enable("y", false);
	dialog->enable("complexity", false);
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
	dialog->event("type:ball", std::bind(&ModeModelMeshCreateBall::onTypeBall, this));
	dialog->event("type:sphere", std::bind(&ModeModelMeshCreateBall::onTypeSphere, this));
	dialog->event("type:physical", std::bind(&ModeModelMeshCreateBall::onTypePhysical, this));
	ed->set_side_panel(dialog);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}


void ModeModelMeshCreateBall::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeModelMeshCreateBall::updateGeometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
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



void ModeModelMeshCreateBall::on_left_button_up()
{
	if (pos_chosen){
		bool physical = dialog->is_checked("type:physical");
		if (physical){
			ModelVertex v;
			v.pos = pos;
			ModelBall b;
			b.index = data->skin[0].vertex.num;
			b.radius = radius;
			data->skin[0].vertex.add(v);
			data->ball.add(b);

		}else{
			data->pasteGeometry(*geo, mode_model_mesh->current_material);
			data->selectOnlySurface(&data->surface.back());
		}

		abort();
	}else{
		pos = multi_view->get_cursor();
		message = _("Kugelradius: ");
		pos_chosen = true;
		updateGeometry();
	}
}


void ModeModelMeshCreateBall::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	if (pos_chosen){
		mode_model->set_material_creation();
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);

		if (win == multi_view->mouse_win)
			draw_helper_line(win, pos, multi_view->get_cursor());
	}
}


void ModeModelMeshCreateBall::on_mouse_move()
{
	if (pos_chosen){
		vector pos2 = multi_view->get_cursor(pos);
		radius = (pos2 - pos).length();
		if (multi_view->snap_to_grid)
			radius = multi_view->snap_f(radius);
		updateGeometry();
		message = _("Kugelradius: ") + multi_view->format_length(radius);
	}
}



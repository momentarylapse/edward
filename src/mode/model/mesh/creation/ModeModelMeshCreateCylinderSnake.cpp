/*
 * ModeModelMeshCreateCylinderSnake.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinderSnake.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/geometry/GeometryCylinder.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../multiview/ColorScheme.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/math/interpolation.h"

const float CYLINDER_CLOSING_DISTANCE = 20;


ModeModelMeshCreateCylinderSnake::ModeModelMeshCreateCylinderSnake(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreateCylinderSnake", _parent)
{
	message = _("Cylinder snake... points + [Ctrl + Return]");

	radius = 0;
	closed = false;
	ready_for_scaling = false;
	geo = NULL;
}

void ModeModelMeshCreateCylinderSnake::on_start() {
	dialog = new hui::Panel();
	dialog->from_resource("new_cylinder_dialog");

	dialog->set_int("rings", hui::config.get_int("NewCylinderRings", 4));
	dialog->set_int("edges", hui::config.get_int("NewCylinderEdges", 8));
	dialog->check("round", hui::config.get_bool("NewCylinderRound", false));
	dialog->hide_control("type:visible", true);
	dialog->hide_control("type:physical", true);
	ed->set_side_panel(dialog);

	bool physical = (parent->current_skin == MESH_PHYSICAL);
	if (physical)
		dialog->enable("*", false);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinderSnake::on_end() {
	ed->set_side_panel(nullptr);
	if (geo)
		delete geo;
}

void ModeModelMeshCreateCylinderSnake::update_geometry() {
	if (geo)
		delete geo;
	if (ready_for_scaling){
		bool round = dialog->is_checked("round");
		int rings = dialog->get_int("rings");
		int edges = dialog->get_int("edges");
		hui::config.set_int("NewCylinderRings", rings);
		hui::config.set_int("NewCylinderEdges", edges);
		hui::config.set_bool("NewCylinderRound", round);

		geo = new GeometryCylinder(pos, radius, rings * (pos.num - 1), edges, closed ? GeometryCylinder::END_LOOP : (round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT));
	}
}


void ModeModelMeshCreateCylinderSnake::on_mouse_move() {
	if (ready_for_scaling) {
		vec3 p = multi_view->get_cursor(pos.back());
		radius = (p - pos.back()).length();
		radius = multi_view->maybe_snap_f(radius);
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		update_geometry();
		message = _("Cylinder radius: ") + multi_view->format_length(radius);
	}
}



void ModeModelMeshCreateCylinderSnake::on_left_button_up() {
	if (ready_for_scaling) {

		data->pasteGeometry(*geo, parent->current_material);

		abort();
	} else {
		if (pos.num > 2) {
			vec2 pp = multi_view->mouse_win->project(pos[0]).xy();
			if ((pp - multi_view->m).length() < CYLINDER_CLOSING_DISTANCE) {
				closed = true;
				ready_for_scaling = true;
				on_mouse_move();
				message = _("Cylinder: radius");
				update_geometry();
				multi_view->force_redraw();
				return;
			}

		}
		pos.add(multi_view->get_cursor());
	}
}



void ModeModelMeshCreateCylinderSnake::on_command(const string& id) {
	if (id == "finish-action") {
		if (pos.num > 1) {
			ready_for_scaling = true;
			on_mouse_move();
			message = _("Cylinder radius: ");
			update_geometry();
			multi_view->force_redraw();
		}
	}
}





void ModeModelMeshCreateCylinderSnake::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos.num > 0) {

		// control points
		win->drawing_helper->set_color(scheme.CREATION_LINE);
		nix::set_shader(win->gl->default_2d.get());
		for (int i=0;i<pos.num;i++) {
			vec3 pp = win->project(pos[i]);
			win->drawing_helper->draw_rect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}


		// control polygon
		win->drawing_helper->set_color(color::interpolate(scheme.CREATION_LINE, scheme.BACKGROUND, 0.3f));
		win->drawing_helper->set_line_width(scheme.LINE_WIDTH_HELPER);
		win->drawing_helper->draw_lines(pos, true);

		if ((!ready_for_scaling) and (pos.num > 0))
			win->drawing_helper->draw_line(pos.back(), multi_view->get_cursor());


		// spline curve
		Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE_NOTANG);
		for (vec3 &p: pos)
			inter.add(p);
		if (!ready_for_scaling)
			inter.add(multi_view->get_cursor());
		inter.normalize();
		win->drawing_helper->set_color(scheme.CREATION_LINE);
		win->drawing_helper->set_line_width(scheme.LINE_WIDTH_HELPER);
		for (int i=0; i<100; i++)
			win->drawing_helper->draw_line(inter.get((float)i * 0.01f), inter.get((float)i * 0.01f + 0.01f));
	}

	if (ready_for_scaling) {
		geo->build(win->gl->vb_temp);
		win->drawing_helper->set_material_creation();
		nix::draw_triangles(win->gl->vb_temp);

		if (win == multi_view->mouse_win)
			win->drawing_helper->draw_helper_line(win, pos.back(), multi_view->get_cursor());
	} else if (pos.num > 2) {
		vec2 pp = multi_view->mouse_win->project(pos[0]).xy();
		if ((pp - multi_view->m).length() < CYLINDER_CLOSING_DISTANCE) {
			nix::set_shader(win->gl->default_2d.get());
			win->drawing_helper->draw_str(pp.x, pp.y, _("Close path"));
		}
	}
}

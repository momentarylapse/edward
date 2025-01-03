/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateCube.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/geometry/GeometryCube.h"
#include "../../../../EdwardWindow.h"
#include "../../../../Session.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/hui/config.h"



ModeModelMeshCreateCube::ModeModelMeshCreateCube(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreateCube", _parent)
{
	message = _("Cube: place first corner point");
	pos_chosen = false;
	pos2_chosen = false;
	for (int i=0;i<3;i++)
		length[i] = v_0;
	geo = nullptr;
}

ModeModelMeshCreateCube::~ModeModelMeshCreateCube(){
	if (geo)
		delete geo;
}

void ModeModelMeshCreateCube::update_geometry() {
	if (geo)
		delete geo;
	if (pos2_chosen) {
		int num_1 = dialog->get_int("nc_x");
		int num_2 = dialog->get_int("nc_y");
		int num_3 = dialog->get_int("nc_z");
		hui::config.set_int("NewCubeNumX", num_1);
		hui::config.set_int("NewCubeNumY", num_2);
		hui::config.set_int("NewCubeNumZ", num_3);

		if (parent->current_skin == MESH_PHYSICAL)
			num_1 = num_2 = num_3 = 1;

		geo = new GeometryCube(pos-length[2]/2, length[0], length[1], length[2], num_1, num_2, num_3);
	} else {
		float min_thick = 10 / multi_view->active_win->zoom(); // 10 px
		vec3 n = vec3::cross(length[0], length[1]);
		n.normalize();
		geo = new GeometryCube(pos, length[0], length[1], n * min_thick, 1, 1, 1);
	}
}


bool ModeModelMeshCreateCube::set_dpos3() {
	vec3 n = vec3::cross(length[0], length[1]).normalized();
	vec3 dpos = multi_view->get_cursor() - pos2;
	float min_thick = 10 / multi_view->active_win->zoom(); // 10 px


	if (fabs(vec3::dot(multi_view->mouse_win->get_edit_direction(), n)) > 0.90f) {
		// cursor in cube plane -> use radius
		length[2] = n * multi_view->maybe_snap_f(max(dpos.length(), min_thick)) * 2;
		if (vec3::dot(multi_view->mouse_win->get_direction(), n) < 0)
			length[2] = -length[2];
		return true;
	}

	length[2] = multi_view->maybe_snap_v(n * vec3::dot(n, dpos)) * 2;
	if (length[2].length() < min_thick)
		length[2] = n * min_thick;
	return false;
}



void ModeModelMeshCreateCube::on_left_button_up() {
	if (pos_chosen) {
		if (pos2_chosen) {

			data->pasteGeometry(*geo, parent->current_material);

			abort();
		} else {
			pos2 = multi_view->get_cursor();
			pos2_chosen = true;
			set_dpos3();
		}
	} else {
		pos = multi_view->get_cursor();
		pos_chosen = true;
	}

	on_mouse_move();
}

void ModeModelMeshCreateCube::on_mouse_move() {
	if (pos_chosen) {
		if (!pos2_chosen) {
			vec3 pos2 = multi_view->get_cursor();
			vec3 dir[3];
			multi_view->mouse_win->get_edit_frame(dir[0], dir[1], dir[2]);
			length[0] = dir[2] * vec3::dot(dir[2], pos2 - pos);
			length[1] = dir[1] * vec3::dot(dir[1], pos2 - pos);
			update_geometry();

			message = _("Cube base area: ") + multi_view->format_length(length[0].length()) + " x " + multi_view->format_length(length[1].length());
		} else {
			set_dpos3();
			update_geometry();

			message = _("Cube: ") + multi_view->format_length(length[0].length()) + " x " + multi_view->format_length(length[1].length()) + " x " + multi_view->format_length(length[2].length());
		}
	}
}



void ModeModelMeshCreateCube::on_start() {
	dialog = new hui::Panel();
	dialog->from_resource("new_cube_dialog");

	dialog->set_int("nc_x", hui::config.get_int("NewCubeNumX", 1));
	dialog->set_int("nc_y", hui::config.get_int("NewCubeNumY", 1));
	dialog->set_int("nc_z", hui::config.get_int("NewCubeNumZ", 1));
	session->win->set_side_panel(dialog);

	bool physical = (parent->current_skin == MESH_PHYSICAL);
	if (physical)
		dialog->enable("*", false);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	session->win->activate("");
}

void ModeModelMeshCreateCube::on_end() {
	session->win->set_side_panel(nullptr);
}

void ModeModelMeshCreateCube::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	win->drawing_helper->set_material_creation();
	if (pos_chosen) {
#if HAS_LIB_GL
		geo->build(win->ctx->vb_temp);
		nix::draw_triangles(win->ctx->vb_temp);
#endif
	}
	if (pos2_chosen and win == multi_view->mouse_win) {
		vec3 m = multi_view->get_cursor();
		if (set_dpos3()) {
			win->drawing_helper->draw_helper_line(win, pos2, m);
		} else {
			vec3 n = vec3::cross(length[0], length[1]).normalized();
			win->drawing_helper->draw_helper_line(win, m + n * vec3::dot(n, pos2-m), m);
		}
	}
}



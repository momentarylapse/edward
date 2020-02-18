/*
 * ModeModelMeshCreateCube.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreateCube.h"
#include "../../../../Data/Model/Geometry/GeometryCube.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"



ModeModelMeshCreateCube::ModeModelMeshCreateCube(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCube", _parent)
{
	message = _("Cube: place first corner point");
	pos_chosen = false;
	pos2_chosen = false;
	for (int i=0;i<3;i++)
		length[i] = v_0;
	geo = NULL;
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
		hui::Config.set_int("NewCubeNumX", num_1);
		hui::Config.set_int("NewCubeNumY", num_2);
		hui::Config.set_int("NewCubeNumZ", num_3);

		if (mode_model_mesh->current_skin == SKIN_PHYSICAL)
			num_1 = num_2 = num_3 = 1;

		geo = new GeometryCube(pos, length[0], length[1], length[2], num_1, num_2, num_3);
	} else {
		float min_thick = 10 / ed->multi_view_3d->active_win->zoom(); // 10 px
		vector n = length[0] ^ length[1];
		n.normalize();
		geo = new GeometryCube(pos, length[0], length[1], n * min_thick, 1, 1, 1);
	}
}


bool ModeModelMeshCreateCube::set_dpos3() {
	vector n = (length[0] ^ length[1]).normalized();
	vector dpos = multi_view->get_cursor() - pos2;
	length[2] = multi_view->maybe_snap_v(n * (n * dpos));
	float min_thick = 10 / ed->multi_view_3d->active_win->zoom(); // 10 px


	if (fabs(multi_view->mouse_win->get_direction() * n) > 0.97f) {
		// cursor in cube plane -> use radius
		length[2] = n * multi_view->maybe_snap_f(max(dpos.length(), min_thick));
		if (multi_view->mouse_win->get_direction() * n < 0)
			length[2] = -length[2];
		return true;
	}

	if (length[2].length() < min_thick)
		length[2] = n * min_thick;
	return false;
}



void ModeModelMeshCreateCube::on_left_button_up() {
	if (pos_chosen) {
		if (pos2_chosen) {

			data->pasteGeometry(*geo, mode_model_mesh->current_material);

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
			vector pos2 = multi_view->get_cursor();
			vector dir0 = multi_view->mouse_win->get_direction_right();
			vector dir1 = multi_view->mouse_win->get_direction_up();
			length[0] = dir0 * vector::dot(dir0, pos2 - pos);
			length[1] = dir1 * vector::dot(dir1, pos2 - pos);
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

	dialog->set_int("nc_x", hui::Config.get_int("NewCubeNumX", 1));
	dialog->set_int("nc_y", hui::Config.get_int("NewCubeNumY", 1));
	dialog->set_int("nc_z", hui::Config.get_int("NewCubeNumZ", 1));
	ed->set_side_panel(dialog);

	bool physical = (mode_model_mesh->current_skin == SKIN_PHYSICAL);
	if (physical)
		dialog->enable("*", false);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}

void ModeModelMeshCreateCube::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeModelMeshCreateCube::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	mode_model->set_material_creation();
	if (pos_chosen) {
		geo->build(nix::vb_temp);
		nix::DrawTriangles(nix::vb_temp);
	}
	if (pos2_chosen and win == multi_view->mouse_win) {
		vector m = multi_view->get_cursor();
		if (set_dpos3()) {
			draw_helper_line(win, pos2, m);
		} else {
			vector n = (length[0] ^ length[1]).normalized();
			draw_helper_line(win, m + n * (n * (pos2-m)), m);
		}
	}
}



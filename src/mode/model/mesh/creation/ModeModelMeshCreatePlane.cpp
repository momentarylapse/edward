/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreatePlane.h"
#include "../../../../data/model/geometry/GeometryPlane.h"
#include "../../../../EdwardWindow.h"
#include "../../../../Session.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/hui/config.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreatePlane", _parent)
{
	message = _("Plane: first point");
	pos_chosen = false;
	length[0] = length[1] = v_0;
	invert = false;
}

void ModeModelMeshCreatePlane::on_start() {
	// Dialog
	dialog = new hui::Panel();
	dialog->from_resource("new_plane_dialog");
	dialog->set_int("np_num_x", hui::config.get_int("NewPlaneNumX", 4));
	dialog->set_int("np_num_y",hui::config.get_int("NewPlaneNumY", 4));
	session->win->set_side_panel(dialog);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}


void ModeModelMeshCreatePlane::on_end() {
	session->win->set_side_panel(nullptr);
}


void ModeModelMeshCreatePlane::on_left_button_up() {
	if (pos_chosen) {
		int nx = dialog->get_int("np_num_x");
		int ny = dialog->get_int("np_num_y");
		hui::config.set_int("NewPlaneNumX", nx);
		hui::config.set_int("NewPlaneNumY", ny);

		if (invert) {
			pos += length[0];
			length[0] = -length[0];
		}

		GeometryPlane pl = GeometryPlane(pos, length[0], length[1], nx, ny);
		data->pasteGeometry(pl, parent->current_material);

		abort();
	} else {
		pos = multi_view->get_cursor();
		message = _("Plane: second point");
		pos_chosen = true;
	}
}



void ModeModelMeshCreatePlane::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
#if HAS_LIB_GL
	if (pos_chosen) {
		auto geo = GeometryPlane(pos, length[0], length[1], 1,1);
		geo.add(GeometryPlane(pos, length[1], length[0], 1,1));
		geo.build(win->ctx->vb_temp);
		win->drawing_helper->set_material_creation();
		nix::draw_triangles(win->ctx->vb_temp);
	}
#endif
}



void ModeModelMeshCreatePlane::on_mouse_move() {
	if (pos_chosen) {
		vec3 pos2 = multi_view->get_cursor();
		vec3 dir0, dir1, dir2;
		multi_view->mouse_win->get_edit_frame(dir0, dir1, dir2);
		length[0] = dir1 * vec3::dot(dir1, pos2 - pos);
		length[1] = dir2 * vec3::dot(dir2, pos2 - pos);
		invert = (vec3::dot(vec3::cross(length[0], length[1]), dir0) > 0);
	}
}

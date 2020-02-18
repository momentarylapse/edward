/*
 * ModeModelMeshCreatePlane.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "ModeModelMeshCreatePlane.h"
#include "../../../../Data/Model/Geometry/GeometryPlane.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshCreatePlane::ModeModelMeshCreatePlane(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePlane", _parent)
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
	dialog->set_int("np_num_x", hui::Config.get_int("NewPlaneNumX", 4));
	dialog->set_int("np_num_y",hui::Config.get_int("NewPlaneNumY", 4));
	ed->set_side_panel(dialog);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}


void ModeModelMeshCreatePlane::on_end() {
	ed->set_side_panel(nullptr);
}


void ModeModelMeshCreatePlane::on_left_button_up()
{
	if (pos_chosen){
		int nx = dialog->get_int("np_num_x");
		int ny = dialog->get_int("np_num_y");
		hui::Config.set_int("NewPlaneNumX", nx);
		hui::Config.set_int("NewPlaneNumY", ny);

		if (invert){
			pos += length[0];
			length[0] = -length[0];
		}

		GeometryPlane pl = GeometryPlane(pos, length[0], length[1], nx, ny);
		data->pasteGeometry(pl, mode_model_mesh->current_material);

		abort();
	}else{
		pos = multi_view->get_cursor();
		message = _("Plane: second point");
		pos_chosen = true;
	}
}



void ModeModelMeshCreatePlane::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos_chosen) {
		auto geo = GeometryPlane(pos, length[0], length[1], 1,1);
		geo.build(nix::vb_temp);
		mode_model->set_material_creation();
		nix::SetCull(CULL_NONE);
		nix::DrawTriangles(nix::vb_temp);
		nix::SetCull(CULL_DEFAULT);
	}
}



void ModeModelMeshCreatePlane::on_mouse_move() {
	if (pos_chosen) {
		vector pos2 = multi_view->get_cursor();
		vector dir0, dir1, dir2;
		multi_view->mouse_win->get_moving_frame(dir0, dir1, dir2);
		length[0] = dir1 * vector::dot(dir1, pos2 - pos);
		length[1] = dir2 * vector::dot(dir2, pos2 - pos);
		invert = (((length[0] ^ length[1]) * dir0) > 0);
	}
}

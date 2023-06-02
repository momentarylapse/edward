/*
 * ModeModelMeshCreateTorus.cpp
 *
 *  Created on: 29.09.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateTorus.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../data/model/geometry/GeometryTorus.h"
#include "../../../../Edward.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"

#define RADIUS_FACTOR	0.5f



ModeModelMeshCreateTorus::ModeModelMeshCreateTorus(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateTorus", _parent)
{
	message = _("Select torus center");

	pos_chosen = false;
	rad_chosen = false;
	radius1 = 0;
	radius2 = 0;
	axis = vec3::EZ;
	geo = NULL;
}

ModeModelMeshCreateTorus::~ModeModelMeshCreateTorus()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateTorus::on_start() {
	// Dialog
	dialog = new hui::Panel();
	dialog->from_resource("new_torus_dialog");

	dialog->set_int("nt_rings", hui::config.get_int("NewTorusNumX", 32));
	dialog->set_int("nt_edges", hui::config.get_int("NewTorusNumY", 16));
	ed->set_side_panel(dialog);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	ed->activate("");
}


void ModeModelMeshCreateTorus::on_end() {
	ed->set_side_panel(nullptr);
}

void ModeModelMeshCreateTorus::update_geometry()
{
	if (geo)
		delete(geo);
	if (pos_chosen){
		int nx = dialog->get_int("nt_rings");
		int ny = dialog->get_int("nt_edges");
		hui::config.set_int("NewTorusNumX", nx);
		hui::config.set_int("NewTorusNumY", ny);
		geo = new GeometryTorus(pos, axis, radius1, radius2, nx, ny);
	}
}


void ModeModelMeshCreateTorus::on_left_button_up()
{
	if (pos_chosen){
		if (rad_chosen){

			data->pasteGeometry(*geo, mode_model_mesh->current_material);

			abort();
		}else{
			message = _("Torus inner scale");
			rad_chosen = true;
		}
	}else{
		pos = multi_view->get_cursor();
		message = _("Torus outer scale");
		pos_chosen = true;
		update_geometry();
	}
}


void ModeModelMeshCreateTorus::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (pos_chosen) {
		set_material_creation();
		geo->build(nix::vb_temp);
		nix::draw_triangles(nix::vb_temp);

		if (win == multi_view->mouse_win) {
			draw_helper_line(win, pos, multi_view->get_cursor());
		}
	}
}



void ModeModelMeshCreateTorus::on_mouse_move()
{
	axis = multi_view->mouse_win->get_direction();
	if (pos_chosen){
		vec3 m = multi_view->get_cursor(pos);
		if (rad_chosen){
			radius2 = (m - pos).length() * RADIUS_FACTOR;
			radius2 = multi_view->maybe_snap_f(radius2);
			message = _("Torus outer scale: ") + multi_view->format_length(radius1) + " / " + multi_view->format_length(radius2);
		}else{
			radius1 = (m - pos).length();
			radius2 = radius1 * RADIUS_FACTOR;
			radius1 = multi_view->maybe_snap_f(radius1);
			radius2 = multi_view->maybe_snap_f(radius2);

			message = _("Torus inner scale:") + multi_view->format_length(radius1);
		}
		update_geometry();
	}
}



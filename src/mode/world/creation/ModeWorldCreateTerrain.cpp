/*
 * ModeWorldCreateTerrain.cpp
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#include "ModeWorldCreateTerrain.h"
#include "../ModeWorld.h"
#include "../../../data/model/geometry/GeometryPlane.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/Window.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../lib/nix/nix.h"

ModeWorldCreateTerrain::ModeWorldCreateTerrain(ModeWorld *_parent) :
	ModeCreation<ModeWorld, DataWorld>("WorldCreateTerrain", _parent)
{
	message = _("Terrain: [click] set first corner");
	pos = v_0;
	pos_chosen = false;
	size = v_0;
}

void ModeWorldCreateTerrain::on_start() {
	// Dialog
	dialog = new hui::Panel();
	dialog->from_resource("world-new-terrain-dialog");
	session->win->set_side_panel(dialog);

	dialog->set_int("stripes-x", 64);
	dialog->set_int("stripes-z", 64);

}

void ModeWorldCreateTerrain::on_end() {
	session->win->set_side_panel(nullptr);
	/*delete(dialog);
	dialog = nullptr;*/
}

void ModeWorldCreateTerrain::on_left_button_up() {
	if (pos_chosen) {
		int nx = dialog->get_int("stripes-x");
		int nz = dialog->get_int("stripes-z");
		//hui::config.set_int("NewTerrainNumX", nx);
		//hui::config.set_int("NewTerrainNumZ", nz);

		if (size.x < 0) {
			size.x = - size.x;
			pos.x -= size.x;
		}
		if (size.z < 0) {
			size.z = - size.z;
			pos.z -= size.z;
		}
		data->add_new_terrain(pos, size, nx, nz);

		abort();
	} else {
		pos = multi_view->get_cursor();
		message = _("Terrain: [click] set opposite corner");
		pos_chosen = true;
	}
}

void ModeWorldCreateTerrain::on_mouse_move() {
	if (pos_chosen) {
		vec3 pos2 = multi_view->get_cursor();
		size.x = (pos2 - pos).x;
		size.z = (pos2 - pos).z;

		dialog->set_float("total-size-x", size.x);
		dialog->set_float("total-size-z", size.z);

		dialog->set_float("grid-size-x", size.x / (float)dialog->get_int("stripes-x"));
		dialog->set_float("grid-size-z", size.z / (float)dialog->get_int("stripes-z"));
	}
}

void ModeWorldCreateTerrain::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

#if HAS_LIB_GL
	if (pos_chosen) {
		auto geo = GeometryPlane(pos, size.x * vec3::EX, size.z * vec3::EZ, 1,1);
		geo.add(GeometryPlane(pos, size.z * vec3::EZ, size.x * vec3::EX, 1,1));
		geo.build(win->ctx->vb_temp);
		win->drawing_helper->set_material_creation();
		nix::draw_triangles(win->ctx->vb_temp);
	}
#endif
}



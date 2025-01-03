/*
 * ModeModelMeshPaste.cpp
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#include "ModeModelMeshPaste.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../stuff/Clipboard.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../Session.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/os/msg.h"

ModeModelMeshPaste::ModeModelMeshPaste(ModeModelMesh* _parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshPaste", _parent)
{
	message = _("move, [left click] to insert");

	vec3 min, max;
	clipboard.temp_geo.get_bounding_box(min, max);
	dpos0 = (max + min) / 2;
}

void ModeModelMeshPaste::on_start() {
	msg_write("on start");
	if (!clipboard.has_mesh_data()) {
		session->set_message(_("nothing to paste"));
		abort();
		return;
	}

	update_geometry();
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshPaste::on_end() {
	geo = nullptr;
}

void ModeModelMeshPaste::on_mouse_move() {
	update_geometry();
}

void ModeModelMeshPaste::on_left_button_up() {
	data->pasteGeometry(*geo, parent->current_material);
	session->set_message(format(_("%d vertices, %d triangles pasted"), geo->vertex.num, geo->polygon.num));
	abort();
}

void ModeModelMeshPaste::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);

	if (geo) {
		win->drawing_helper->set_material_creation();
#if HAS_LIB_GL
		geo->build(win->ctx->vb_temp);
		nix::draw_triangles(win->ctx->vb_temp);
#endif
	}
}

void ModeModelMeshPaste::update_geometry() {
	msg_write("update");
	mat4 m;
	m = mat4::translation( multi_view->get_cursor() - dpos0);
	geo = new Geometry;
	geo->add(clipboard.temp_geo);
	geo->transform(m);
}

/*
 * ModeModelMeshPaste.cpp
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#include "ModeModelMeshPaste.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshPaste::ModeModelMeshPaste(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshPaste", _parent)
{
	message = _("move, [left click] to insert");
	geo = nullptr;

	vector min, max;
	mode_model_mesh->temp_geo.get_bounding_box(min, max);
	dpos0 = (max + min) / 2;
}

void ModeModelMeshPaste::on_start() {
	msg_write("on start");
	if (mode_model_mesh->temp_geo.vertex.num == 0) {
		ed->set_message(_("nothing to paste"));
		abort();
		return;
	}

	update_geometry();
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshPaste::on_end() {
	if (geo)
		delete geo;
}

void ModeModelMeshPaste::on_mouse_move() {
	update_geometry();
}

void ModeModelMeshPaste::on_left_button_up() {
	data->pasteGeometry(*geo, mode_model_mesh->current_material);
	ed->set_message(format(_("%d vertices, %d triangles pasted"), geo->vertex.num, geo->polygon.num));
	abort();
}

void ModeModelMeshPaste::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);

	if (geo) {
		set_material_creation();
		geo->build(nix::vb_temp);
		nix::draw_triangles(nix::vb_temp);
	}
}

void ModeModelMeshPaste::update_geometry() {
	msg_write("update");
	if (geo)
		delete geo;
	matrix m;
	m = matrix::translation( multi_view->get_cursor() - dpos0);
	geo = new Geometry;
	geo->add(mode_model_mesh->temp_geo);
	geo->transform(m);
}

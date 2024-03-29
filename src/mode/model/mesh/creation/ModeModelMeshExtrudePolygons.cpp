/*
 * ModeModelMeshExtrudePolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ModeModelMeshExtrudePolygons.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../action/model/mesh/polygon/ActionModelExtrudePolygons.h"

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(ModeModelMesh *_parent, bool _independent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshExtrudePolygons", _parent)
{
	selection = data->get_selection();

	independent = _independent;
	offset = 0;

	message = _("Extrude: offset via mouse, left button = done");
}

void ModeModelMeshExtrudePolygons::on_end() {
	cleanUp();
}

void ModeModelMeshExtrudePolygons::on_left_button_down() {
	cleanUp();

	data->set_selection(selection);
	data->execute(new ActionModelExtrudePolygons(offset, independent, parent->current_material));

	abort();
}

void ModeModelMeshExtrudePolygons::on_mouse_move() {
	cleanUp();

	offset += (hui::get_event()->d.x) / multi_view->active_win->zoom();
	preview();

	message = _("Extrude: Offset via mouse, left click = done" + multi_view->format_length(offset));
}

void ModeModelMeshExtrudePolygons::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);
}

void ModeModelMeshExtrudePolygons::preview() {
	data->set_selection(selection);
	if (!data->action_manager->preview(new ActionModelExtrudePolygons(offset, independent, parent->current_material)))
		abort();
}

void ModeModelMeshExtrudePolygons::on_start() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshExtrudePolygons::cleanUp() {
	data->action_manager->clear_preview();
}


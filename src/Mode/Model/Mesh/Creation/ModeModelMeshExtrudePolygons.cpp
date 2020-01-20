/*
 * ModeModelMeshExtrudePolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ModeModelMeshExtrudePolygons.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.h"

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(ModeBase *_parent, bool _independent) :
	ModeCreation<DataModel>("ModelMeshExtrudePolygons", _parent)
{
	data->getSelectionState(selection);

	independent = _independent;
	offset = 0;

	message = _("Extrude: offset via mouse, left button = done");
}

void ModeModelMeshExtrudePolygons::on_end() {
	cleanUp();
}

void ModeModelMeshExtrudePolygons::on_left_button_down() {
	cleanUp();

	data->setSelectionState(selection);
	data->execute(new ActionModelExtrudePolygons(offset, independent));

	abort();
}

void ModeModelMeshExtrudePolygons::on_mouse_move() {
	cleanUp();

	offset += (hui::GetEvent()->dx) / multi_view->active_win->zoom();
	preview();

	message = _("Extrude: Offset via mouse, left click = done" + multi_view->format_length(offset));
}

void ModeModelMeshExtrudePolygons::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	nix::EnableLighting(false);
}

void ModeModelMeshExtrudePolygons::preview() {
	data->setSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelExtrudePolygons(offset, independent)))
		abort();
}

void ModeModelMeshExtrudePolygons::on_start() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshExtrudePolygons::cleanUp() {
	data->action_manager->clear_preview();
}


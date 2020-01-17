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

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshExtrudePolygons", _parent)
{
	data->getSelectionState(selection);

	offset = 0;

	message = _("Extrudieren: Offset durch Maus, Linke Taste = fertig");
}

void ModeModelMeshExtrudePolygons::on_end()
{
	cleanUp();
}

void ModeModelMeshExtrudePolygons::on_left_button_down()
{
	cleanUp();

	data->setSelectionState(selection);
	data->execute(new ActionModelExtrudePolygons(offset));

	abort();
}

void ModeModelMeshExtrudePolygons::on_mouse_move()
{
	cleanUp();

	offset += (hui::GetEvent()->dx) / multi_view->active_win->zoom();
	preview();
}

void ModeModelMeshExtrudePolygons::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	nix::EnableLighting(false);
	ed->draw_str(100, 100, f2s(offset, 3));
}

void ModeModelMeshExtrudePolygons::preview()
{
	data->setSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelExtrudePolygons(offset)))
		abort();
}

void ModeModelMeshExtrudePolygons::on_start()
{
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshExtrudePolygons::cleanUp()
{
	data->action_manager->clear_preview();
}


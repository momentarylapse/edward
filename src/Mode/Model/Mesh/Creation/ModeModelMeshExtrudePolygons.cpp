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

void ModeModelMeshExtrudePolygons::onEnd()
{
	cleanUp();
}

void ModeModelMeshExtrudePolygons::onLeftButtonDown()
{
	cleanUp();

	data->setSelectionState(selection);
	data->execute(new ActionModelExtrudePolygons(offset));

	abort();
}

void ModeModelMeshExtrudePolygons::onMouseMove()
{
	cleanUp();

	offset += (hui::GetEvent()->dx) / multi_view->active_win->zoom();
	preview();
}

void ModeModelMeshExtrudePolygons::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	nix::EnableLighting(false);
	ed->drawStr(100, 100, f2s(offset, 3));
}

void ModeModelMeshExtrudePolygons::preview()
{
	data->setSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelExtrudePolygons(offset)))
		abort();
}

void ModeModelMeshExtrudePolygons::onStart()
{
	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);
}

void ModeModelMeshExtrudePolygons::cleanUp()
{
	data->action_manager->clearPreview();
}


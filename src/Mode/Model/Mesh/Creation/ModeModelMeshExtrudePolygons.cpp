/*
 * ModeModelMeshExtrudePolygons.cpp
 *
 *  Created on: 28.09.2012
 *      Author: michi
 */

#include "ModeModelMeshExtrudePolygons.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelExtrudePolygons.h"

ModeModelMeshExtrudePolygons::ModeModelMeshExtrudePolygons(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshExtrudePolygons", _parent)
{
	data->GetSelectionState(selection);

	offset = 0;

	message = _("Extrudieren: Offset durch Maus, Linke Taste = fertig");
}

ModeModelMeshExtrudePolygons::~ModeModelMeshExtrudePolygons()
{
}

void ModeModelMeshExtrudePolygons::onEnd()
{
	CleanUp();
}

void ModeModelMeshExtrudePolygons::onLeftButtonDown()
{
	CleanUp();

	data->SetSelectionState(selection);
	data->execute(new ActionModelExtrudePolygons(offset));

	abort();
}

void ModeModelMeshExtrudePolygons::onMouseMove()
{
	CleanUp();

	offset += (HuiGetEvent()->dx) / multi_view->cam.zoom;
	Preview();
}

void ModeModelMeshExtrudePolygons::onDrawWin(MultiView::Window *win)
{
	NixEnableLighting(false);
	ed->drawStr(100, 100, f2s(offset, 3));
}

void ModeModelMeshExtrudePolygons::Preview()
{
	data->SetSelectionState(selection);
	if (!data->action_manager->preview(new ActionModelExtrudePolygons(offset)))
		abort();
}

void ModeModelMeshExtrudePolygons::CleanUp()
{
	data->action_manager->clearPreview();
}


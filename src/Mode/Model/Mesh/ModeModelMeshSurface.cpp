/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshSurface.h"
#include "ModeModelMeshPolygon.h"

ModeModelMeshSurface *mode_model_mesh_surface = NULL;


ModeModelMeshSurface::ModeModelMeshSurface(ModeBase *_parent) :
	Mode("ModelMeshSurface", _parent, ed->multi_view_3d, "menu_model")
{
}



void ModeModelMeshSurface::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh_polygon->onDrawWin(win);
}



void ModeModelMeshSurface::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
}

bool ModelSurface::hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data)
{
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].hover(win, m, tp, z, user_data))
			return true;
	return false;
}

bool ModelSurface::inRect(MultiView::Window *win, rect &r, void *user_data)
{
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].inRect(win, r, user_data))
			return true;
	return false;
}

void ModeModelMeshSurface::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->AddData(	MVDModelSurface,
				data->surface,
				data,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o == multi_view){
		data->SelectionFromSurfaces();
	}
	mode_model_mesh_polygon->fillSelectionBuffers(data->vertex);
}



void ModeModelMeshSurface::onStart()
{
	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	mode_model_mesh->applyMouseFunction(multi_view);
	multi_view->allow_rect = true;
	onUpdate(data, "");
}



void ModeModelMeshSurface::onDraw()
{
	mode_model_mesh_polygon->fillSelectionBuffers(data->vertex);
}



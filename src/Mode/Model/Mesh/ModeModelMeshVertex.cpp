/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshPolygon.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../../../lib/nix/nix.h"

ModeModelMeshVertex *mode_model_mesh_vertex = NULL;

ModeModelMeshVertex::ModeModelMeshVertex(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshVertex", _parent, ed->multi_view_3d, "menu_model")
{
}

void ModeModelMeshVertex::onStart()
{
	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	mode_model_mesh->applyMouseFunction(multi_view);
	multi_view->allow_rect = true;
	onUpdate(data, "");
}

void ModeModelMeshVertex::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
}



void ModeModelMeshVertex::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh_polygon->onDrawWin(win);
	mode_model_skeleton->drawSkeleton(win, data->bone, true);
	drawEffects(win);
}



void ModeModelMeshVertex::onDraw()
{
}

void ModeModelMeshVertex::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->AddData(	MVDModelVertex,
				data->vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o == multi_view){
		data->SelectionFromVertices();
	}
	mode_model_mesh_polygon->fillSelectionBuffers(data->vertex);
}

void ModeModelMeshVertex::drawEffects(MultiView::Window *win)
{
	NixEnableLighting(false);
	foreach(ModelEffect &fx, data->fx){
		vector p = win->project(data->vertex[fx.vertex].pos);
		if ((p.z > 0) && (p.z < 1))
			ed->drawStr(p.x, p.y, fx.get_type());
	}
	NixEnableLighting(multi_view->light_enabled);
}


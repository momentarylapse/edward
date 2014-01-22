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

ModeModelMeshSurface::~ModeModelMeshSurface()
{
}



void ModeModelMeshSurface::OnDrawWin(MultiView::Window *win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
}



void ModeModelMeshSurface::OnEnd()
{
	multi_view->ResetData(data);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}

bool ModelSurface::Hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data)
{
	for (int i=0;i<Polygon.num;i++)
		if (Polygon[i].Hover(win, m, tp, z, user_data))
			return true;
	return false;
}

bool ModelSurface::InRect(MultiView::Window *win, rect &r, void *user_data)
{
	for (int i=0;i<Polygon.num;i++)
		if (Polygon[i].InRect(win, r, user_data))
			return true;
	return false;
}

void ModeModelMeshSurface::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->SetData(	MVDModelSurface,
				data->Surface,
				data,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromSurfaces();
	}
	mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
}



void ModeModelMeshSurface::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->allow_rect = true;
	OnUpdate(data);
}



void ModeModelMeshSurface::OnDraw()
{
	mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
}



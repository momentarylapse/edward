/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
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



void ModeModelMeshSurface::OnDrawWin(MultiViewWindow *win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
}



void ModeModelMeshSurface::OnEnd()
{
	multi_view->ResetData(data);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}

bool PolygonIsMouseOver(int index, void *user_data, MultiViewWindow *win, vector &tp);
bool PolygonInRect(int index, void *user_data, MultiViewWindow *win, rect *r);

bool SurfaceIsMouseOver(int index, void *user_data, MultiViewWindow *win, vector &tp)
{
	DataModel *data = (DataModel*)user_data;
	ModelSurface &s = data->Surface[index];
	for (int i=0;i<s.Polygon.num;i++)
		if (PolygonIsMouseOver(i, &s, win, tp))
			return true;
	return false;
}

bool SurfaceInRect(int index, void *user_data, MultiViewWindow *win, rect *r)
{
	DataModel *data = (DataModel*)user_data;
	ModelSurface &s = data->Surface[index];
	for (int i=0;i<s.Polygon.num;i++)
		if (!PolygonInRect(i, &s, win, r))
			return false;
	return true;
}

void ModeModelMeshSurface::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->SetData(	MVDModelSurface,
				data->Surface,
				data,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&SurfaceIsMouseOver, &SurfaceInRect);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromSurfaces();
	}
	mode_model_mesh_polygon->FillSelectionBuffers();
}



void ModeModelMeshSurface::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->MVRectable = true;
	OnUpdate(data);
}



void ModeModelMeshSurface::OnDraw()
{
	mode_model_mesh_polygon->FillSelectionBuffers();
}



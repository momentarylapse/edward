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
#include "ModeModelMeshTriangle.h"

ModeModelMeshSurface *mode_model_mesh_surface = NULL;


ModeModelMeshSurface::ModeModelMeshSurface(Mode *_parent, DataModel *_data) :
	Mode("ModelMeshSurface", _parent, _data, ed->multi_view_3d, "menu_model")
{
	data = _data;
}

ModeModelMeshSurface::~ModeModelMeshSurface()
{
}



void ModeModelMeshSurface::OnDrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->OnDrawWin(win, dest);
}



void ModeModelMeshSurface::OnEnd()
{
	multi_view->ResetData(data);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}

bool TriangleIsMouseOver(int index, void *user_data, int win, vector &tp);
bool TriangleInRect(int index, void *user_data, int win, irect *r);

bool SurfaceIsMouseOver(int index, void *user_data, int win, vector &tp)
{
	DataModel *data = (DataModel*)user_data;
	ModeModelSurface &s = data->Surface[index];
	for (int i=0;i<s.Triangle.num;i++)
		if (TriangleIsMouseOver(i, &s, win, tp))
			return true;
	return false;
}

bool SurfaceInRect(int index, void *user_data, int win, irect *r)
{
	DataModel *data = (DataModel*)user_data;
	ModeModelSurface &s = data->Surface[index];
	for (int i=0;i<s.Triangle.num;i++)
		if (!TriangleInRect(i, &s, win, r))
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
		data->SelectionVerticesFromSurfaces();
		data->SelectionTrianglesFromSurfaces();
	}
	mode_model_mesh_triangle->FillSelectionBuffers();
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
	mode_model_mesh_triangle->FillSelectionBuffers();
}



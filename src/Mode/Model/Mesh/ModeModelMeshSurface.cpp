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


ModeModelMeshSurface::ModeModelMeshSurface(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshSurface";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}

ModeModelMeshSurface::~ModeModelMeshSurface()
{
}



void ModeModelMeshSurface::DrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->DrawWin(win, dest);
}



void ModeModelMeshSurface::End()
{
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
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDModelSurface,
				data->Surface,
				data,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&SurfaceIsMouseOver, &SurfaceInRect);
	}else if (o->GetName() == "MultiView"){
		// vertex selection from surfaces
		foreach(data->Vertex, v)
			if (v.Surface >= 0)
				v.is_selected = data->Surface[v.Surface].is_selected;
			else
				v.is_selected = false;
		// trias from surfaces
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
				t.is_selected = s.is_selected;
	}
	mode_model_mesh_triangle->FillSelectionBuffers();
}



void ModeModelMeshSurface::Start()
{
	OnUpdate(data);
}



void ModeModelMeshSurface::Draw()
{
	mode_model_mesh_triangle->FillSelectionBuffers();


	if (data->GetNumMarkedVertices() > 0){
		NixDrawStr(20, 100, format(_("vert: %d"), data->GetNumMarkedVertices()));
		NixDrawStr(20, 120, format(_("tria: %d"), data->GetNumMarkedTriangles()));
		NixDrawStr(20, 140, format(_("surf: %d"), data->GetNumMarkedSurfaces()));
	}
}


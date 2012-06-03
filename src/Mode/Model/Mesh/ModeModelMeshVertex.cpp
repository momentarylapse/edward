/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshTriangle.h"
#include "../../../lib/nix/nix.h"

ModeModelMeshVertex *mode_model_mesh_vertex = NULL;

ModeModelMeshVertex::ModeModelMeshVertex(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshVertex";
	parent = _parent;
	data = _data;
	multi_view = ed->multi_view_3d;
	menu = HuiCreateResourceMenu("menu_model");
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::Start()
{
	OnUpdate(data);
}

void ModeModelMeshVertex::End()
{
}



void ModeModelMeshVertex::DrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->DrawWin(win, dest);
}



void ModeModelMeshVertex::Draw()
{
	if (data->GetNumMarkedVertices() > 0){
		NixDrawStr(20, 100, format(_("vert: %d"), data->GetNumMarkedVertices()));
		NixDrawStr(20, 120, format(_("tria: %d"), data->GetNumMarkedTriangles()));
		NixDrawStr(20, 140, format(_("surf: %d"), data->GetNumMarkedSurfaces()));
	}
}

void ModeModelMeshVertex::OnUpdate(Observable *o)
{
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
		// tria selection from vertices
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
				t.is_selected = ((data->Vertex[t.Vertex[0]].is_selected) and (data->Vertex[t.Vertex[1]].is_selected) and (data->Vertex[t.Vertex[2]].is_selected));
		// surface selection from trias
		foreach(data->Surface, s){
			s.is_selected = true;
			foreach(s.Triangle, t)
				s.is_selected &= t.is_selected;
		}
	}
	mode_model_mesh_triangle->FillSelectionBuffers();
}



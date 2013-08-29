/*
 * ModeModelMeshEdge.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshEdge.h"
#include "ModeModelMeshPolygon.h"

ModeModelMeshEdge *mode_model_mesh_edge = NULL;

ModeModelMeshEdge::ModeModelMeshEdge(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshEdge", _parent, ed->multi_view_3d, "menu_model")
{
}



ModeModelMeshEdge::~ModeModelMeshEdge()
{
}

void ModeModelMeshEdge::OnRightButtonUp()
{
}



void ModeModelMeshEdge::OnKeyDown()
{
}



void ModeModelMeshEdge::OnRightButtonDown()
{
}



void ModeModelMeshEdge::OnMiddleButtonDown()
{
}



void ModeModelMeshEdge::OnMiddleButtonUp()
{
}



void ModeModelMeshEdge::OnKeyUp()
{
}



void ModeModelMeshEdge::OnUpdateMenu()
{
}



void ModeModelMeshEdge::OnDraw()
{
}



void ModeModelMeshEdge::OnMouseMove()
{
}



void ModeModelMeshEdge::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->MVRectable = true;
	OnUpdate(data);
}


bool EdgeIsMouseOver(int index, void *user_data, MultiViewWindow *win, vector &tp)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelEdge *e = &surf->Edge[index];

	DataModel *m = mode_model_mesh_edge->data; // surf->model;

	// project all points
	vector pp0 = win->Project(m->Vertex[e->Vertex[0]].pos);
	if ((pp0.z <= 0) or (pp0.z >= 1))
		return false;
	vector pp1 = win->Project(m->Vertex[e->Vertex[1]].pos);
	if ((pp1.z <= 0) or (pp1.z >= 1))
		return false;

	return false;
}

inline bool in_irect(const vector &p, rect *r)
{
	return ((p.x > r->x1) and (p.x < r->x2) and (p.y > r->y1) and (p.y < r->y2));
}

bool EdgeInRect(int index, void *user_data, MultiViewWindow *win, rect *r)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelEdge *e = &surf->Edge[index];

	DataModel *m = mode_model_mesh_edge->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<2;k++){
		vector pp = win->Project(m->Vertex[e->Vertex[k]].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!in_irect(pp, r))
			return false;
	}
	return true;
}


void ModeModelMeshEdge::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		foreach(ModelSurface &s, data->Surface)
		multi_view->SetData(	MVDModelEdge,
				s.Edge,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&EdgeIsMouseOver, &EdgeInRect);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromEdges();
	}
	mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
}



void ModeModelMeshEdge::OnCommand(const string & id)
{
}



void ModeModelMeshEdge::OnLeftButtonDown()
{
}


void ModeModelMeshEdge::DrawEdges(MultiViewWindow *win, Array<ModelVertex> &vertex, bool only_selected)
{
	NixSetWire(false);
	NixEnableLighting(false);
	vector dir = win->GetDirection();
	foreach(ModelSurface &s, data->Surface){
		foreach(ModelEdge &e, s.Edge){
			if (min(vertex[e.Vertex[0]].view_stage, vertex[e.Vertex[1]].view_stage) < multi_view->view_stage)
				continue;
			if (e.is_selected){
				NixSetColor(Red);
			}else if (!only_selected){
				float w = max(s.Polygon[e.Polygon[0]].TempNormal * dir, s.Polygon[e.Polygon[1]].TempNormal * dir);
				float f = 0.7f - 0.3f * w;
				NixSetColor(color(1, f, f, f));
			}else
				continue;
			NixDrawLine3D(vertex[e.Vertex[0]].pos, vertex[e.Vertex[1]].pos);
		}
	}
	NixSetColor(White);
	NixSetWire(true);
	NixEnableLighting(multi_view->light_enabled);
}


void ModeModelMeshEdge::OnDrawWin(MultiViewWindow *win)
{
	if (!multi_view->wire_mode){
		mode_model_mesh_polygon->DrawPolygons(win, data->Vertex);
		mode_model_mesh_polygon->DrawSelection(win);
	}
	DrawEdges(win, data->Vertex, false);
}



void ModeModelMeshEdge::OnEnd()
{
	multi_view->ResetData(NULL);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshEdge::OnLeftButtonUp()
{
}



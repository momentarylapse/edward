/*
 * ModeModelMeshEdge.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshEdge.h"
#include "ModeModelMeshPolygon.h"

ModeModelMeshEdge *mode_model_mesh_edge = NULL;

namespace MultiView{
	extern color ColorBackGround2D;
	extern color ColorText;
};

ModeModelMeshEdge::ModeModelMeshEdge(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshEdge", _parent, ed->multi_view_3d, "menu_model")
{
}



ModeModelMeshEdge::~ModeModelMeshEdge()
{
}

void ModeModelMeshEdge::onRightButtonUp()
{
}



void ModeModelMeshEdge::onKeyDown()
{
}



void ModeModelMeshEdge::onRightButtonDown()
{
}



void ModeModelMeshEdge::onMiddleButtonDown()
{
}



void ModeModelMeshEdge::onMiddleButtonUp()
{
}



void ModeModelMeshEdge::onKeyUp()
{
}



void ModeModelMeshEdge::onUpdateMenu()
{
}



void ModeModelMeshEdge::onDraw()
{
}



void ModeModelMeshEdge::onMouseMove()
{
}



void ModeModelMeshEdge::onStart()
{
	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	mode_model_mesh->applyMouseFunction(multi_view);
	multi_view->allow_rect = true;
	onUpdate(data, "");
}


bool ModelEdge::hover(MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data)
{
	ModelSurface *surf = (ModelSurface*)user_data;

	DataModel *m = mode_model_mesh_edge->data; // surf->model;

	// project all points
	vector pp0 = win->project(m->Vertex[Vertex[0]].pos);
	if ((pp0.z <= 0) or (pp0.z >= 1))
		return false;
	vector pp1 = win->project(m->Vertex[Vertex[1]].pos);
	if ((pp1.z <= 0) or (pp1.z >= 1))
		return false;
	const float rr = 5;
	rect r = rect(min(pp0.x, pp1.x) - rr, max(pp0.x, pp1.x) + rr, min(pp0.y, pp1.y) - rr, max(pp0.y, pp1.y) + rr);
	if (!r.inside(M.x, M.y))
		return false;

	float z0 = pp0.z;
	float z1 = pp1.z;
	pp0.z = pp1.z = 0;
	vector d = pp1 - pp0;
	float l = d.length();
	if (l < 2)
		return false;
	d /= l;
	vector d2 = vector(d.y, -d.x, 0);
	float dd = fabs(d2 * (M - pp0));
	if (dd > rr)
		return false;

	float f = (pp0 + d * ((M - pp0) * d)).factor_between(pp0, pp1);
	tp = m->Vertex[Vertex[0]].pos * (1 - f) + m->Vertex[Vertex[1]].pos * f;
	z = z0 * (1 - f) + z1 * f;
	return true;
}

bool ModelEdge::inRect(MultiView::Window *win, rect &r, void *user_data)
{
	ModelSurface *surf = (ModelSurface*)user_data;

	DataModel *m = mode_model_mesh_edge->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<2;k++){
		vector pp = win->project(m->Vertex[Vertex[k]].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!r.inside(pp.x, pp.y))
			return false;
	}
	return true;
}


void ModeModelMeshEdge::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		foreach(ModelSurface &s, data->Surface)
		multi_view->AddData(	MVDModelEdge,
				s.Edge,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o == multi_view){
		data->SelectionFromEdges();
	}
	mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
}



void ModeModelMeshEdge::onCommand(const string & id)
{
}



void ModeModelMeshEdge::onLeftButtonDown()
{
}


void ModeModelMeshEdge::DrawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected)
{
	NixSetWire(false);
	NixEnableLighting(false);
	vector dir = win->getDirection();
	foreach(ModelSurface &s, data->Surface){
		foreach(ModelEdge &e, s.Edge){
			if (min(vertex[e.Vertex[0]].view_stage, vertex[e.Vertex[1]].view_stage) < multi_view->view_stage)
				continue;
			if (!e.is_selected && only_selected)
				continue;
			float w = max(s.Polygon[e.Polygon[0]].TempNormal * dir, s.Polygon[e.Polygon[1]].TempNormal * dir);
			float f = 0.5f - 0.4f*w;//0.7f - 0.3f * w;
			if (e.is_selected)
				NixSetColor(color(1, f, 0, 0));
			else
				NixSetColor(f * MultiView::ColorText + (1 - f) * MultiView::ColorBackGround2D);
			NixDrawLine3D(vertex[e.Vertex[0]].pos, vertex[e.Vertex[1]].pos);
		}
	}
	NixSetColor(White);
	NixSetWire(win->multi_view->wire_mode);
	NixEnableLighting(multi_view->light_enabled);
}

void ModeModelMeshEdge::onDrawWin(MultiView::Window *win)
{
	if (!multi_view->wire_mode)
		mode_model_mesh_polygon->DrawPolygons(win, data->Vertex);
	DrawEdges(win, data->Vertex, false);
	mode_model_mesh_polygon->DrawSelection(win);

	if (multi_view->hover.index >= 0){
		NixSetWire(false);
		NixSetZ(false, false);
		NixEnableLighting(false);
		NixSetColor(color(1, 0.7f, 0.7f, 1));
		ModelEdge &e = data->Surface[multi_view->hover.set].Edge[multi_view->hover.index];
		NixDrawLine3D(data->Vertex[e.Vertex[0]].pos, data->Vertex[e.Vertex[1]].pos);
		NixSetColor(White);
		NixSetZ(true, true);
		NixSetWire(win->multi_view->wire_mode);
		NixEnableLighting(multi_view->light_enabled);
	}
}



void ModeModelMeshEdge::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
}



void ModeModelMeshEdge::onLeftButtonUp()
{
}



/*
 * ModeModelMeshEdge.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeEdge.h"

#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../ModeModelMesh.h"
#include "../../Skeleton/ModeModelSkeleton.h"
#include "MeshSelectionModePolygon.h"

MeshSelectionModeEdge::MeshSelectionModeEdge(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}


void MeshSelectionModeEdge::onStart()
{
}


bool ModelEdge::hover(MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data)
{
	ModelSurface *surf = (ModelSurface*)user_data;

	DataModel *m = mode_model_mesh->data; // surf->model;

	// project all points
	vector pp0 = win->project(m->show_vertices[vertex[0]].pos);
	if ((pp0.z <= 0) or (pp0.z >= 1))
		return false;
	vector pp1 = win->project(m->show_vertices[vertex[1]].pos);
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
	tp = m->show_vertices[vertex[0]].pos * (1 - f) + m->show_vertices[vertex[1]].pos * f;
	z = z0 * (1 - f) + z1 * f;
	return true;
}

bool ModelEdge::inRect(MultiView::Window *win, rect &r, void *user_data)
{
	ModelSurface *surf = (ModelSurface*)user_data;

	DataModel *m = mode_model_mesh->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<2;k++){
		vector pp = win->project(m->show_vertices[vertex[k]].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!r.inside(pp.x, pp.y))
			return false;
	}
	return true;
}

void MeshSelectionModeEdge::updateSelection()
{
	data->selectionFromEdges();
}

void MeshSelectionModeEdge::updateMultiView()
{
	multi_view->clearData(data);
	multi_view->setAllowSelect(true);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	for (ModelSurface &s: data->surface)
		multi_view->addData(	MVD_MODEL_EDGE,
			s.edge,
			&s,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

void MeshSelectionModeEdge::onDrawWin(MultiView::Window *win)
{
	if (multi_view->hover.type != MVD_MODEL_EDGE)
		return;
	if (multi_view->hover.index < 0)
		return;

	nix::SetWire(false);
	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetColor(color(1, 0.7f, 0.7f, 1));
	ModelEdge &e = data->surface[multi_view->hover.set].edge[multi_view->hover.index];
	nix::DrawLine3D(data->show_vertices[e.vertex[0]].pos, data->show_vertices[e.vertex[1]].pos);
	nix::SetColor(White);
	nix::SetZ(true, true);
	nix::SetWire(win->multi_view->wire_mode);
	nix::EnableLighting(multi_view->light_enabled);
}



void MeshSelectionModeEdge::onEnd()
{
}


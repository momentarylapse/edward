/*
 * ModeModelMeshEdge.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeEdge.h"
#include "MeshSelectionModePolygon.h"
#include "../ModeModelMesh.h"
#include "../../skeleton/ModeModelSkeleton.h"
#include "../../ModeModel.h"
#include "../../../../data/model/ModelMesh.h"
#include "../../../../data/model/ModelPolygon.h"
#include "../../../../EdwardWindow.h"
#include "../../../../Session.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../multiview/ColorScheme.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/math/vec2.h"


MeshSelectionModeEdge::MeshSelectionModeEdge(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}


void MeshSelectionModeEdge::on_start() {
}


float ModelEdge::hover_distance(MultiView::Window *win, const vec2 &M, vec3 &tp, float &z) {

	auto *m = win->multi_view->session->mode_model->mode_model_mesh->data->edit_mesh; // surf->model;

	// project all points
	vec3 pp0 = win->project(m->show_vertices[vertex[0]].pos);
	if ((pp0.z <= 0) or (pp0.z >= 1))
		return -1;
	vec3 pp1 = win->project(m->show_vertices[vertex[1]].pos);
	if ((pp1.z <= 0) or (pp1.z >= 1))
		return -1;
	const float rr = win->multi_view->HOVER_RADIUS;
	rect r = rect(min(pp0.x, pp1.x) - rr, max(pp0.x, pp1.x) + rr, min(pp0.y, pp1.y) - rr, max(pp0.y, pp1.y) + rr);
	if (!r.inside(M))
		return -1;

	//VecLineNearestPoint()

	float z0 = pp0.z;
	float z1 = pp1.z;
	pp0.z = pp1.z = 0;
	vec3 d = pp1 - pp0;
	float l = d.length();
	if (l < 2)
		return -1;
	d /= l;
	vec3 d2 = vec3(d.y, -d.x, 0);
	float dd = fabs(vec3::dot(d2, vec3(M.x,M.y,0) - pp0));
	if (dd > rr)
		return -1;

	float f = (pp0 + d * vec3::dot(vec3(M.x,M.y,0) - pp0, d)).factor_between(pp0, pp1);
	tp = m->show_vertices[vertex[0]].pos * (1 - f) + m->show_vertices[vertex[1]].pos * f;
	z = z0 * (1 - f) + z1 * f;
	return dd;
}

bool ModelEdge::in_rect(MultiView::Window *win, const rect &r) {
	auto *m = win->multi_view->session->mode_model->mode_model_mesh->data->edit_mesh; // surf->model;

	// all vertices within rectangle?
	for (int k=0; k<2; k++) {
		vec3 pp = win->project(m->show_vertices[vertex[k]].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!r.inside({pp.x, pp.y}))
			return false;
	}
	return true;
}

bool ModelEdge::overlap_rect(MultiView::Window *win, const rect &r) {
	return in_rect(win, r);
}

void MeshSelectionModeEdge::on_update_selection() {
	data->edit_mesh->selection_from_edges();
}

void MeshSelectionModeEdge::on_view_stage_change() {
	for (auto &v: data->mesh->vertex)
		if (v.is_selected)
			v.view_stage = multi_view->view_stage;
		else
			v.view_stage = min(v.view_stage, multi_view->view_stage);
	for (auto &p: data->mesh->polygon)
		if (p.is_selected)
			p.view_stage = multi_view->view_stage;
		else
			p.view_stage = min(p.view_stage, multi_view->view_stage);

}

void MeshSelectionModeEdge::update_multi_view() {
	multi_view->clear_data(data);
	multi_view->add_data(MVD_MODEL_EDGE,
			data->edit_mesh->edge,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

void MeshSelectionModeEdge::on_draw_win(MultiView::Window *win) {
	if (multi_view->hover.type != MVD_MODEL_EDGE)
		return;
	if (multi_view->hover.index < 0)
		return;

	nix::set_z(false, false);
	win->drawing_helper->set_line_width(scheme.LINE_WIDTH_MEDIUM);
	auto m = data->edit_mesh;
	auto &e = m->edge[multi_view->hover.index];
	Array<vec3> p;
	p.add(m->show_vertices[e.vertex[0]].pos);
	p.add(m->show_vertices[e.vertex[1]].pos);
	Array<color> c = {scheme.HOVER, scheme.HOVER};
	win->drawing_helper->draw_lines_colored(p, c, false);
	nix::set_z(true, true);
}



void MeshSelectionModeEdge::on_end() {
}


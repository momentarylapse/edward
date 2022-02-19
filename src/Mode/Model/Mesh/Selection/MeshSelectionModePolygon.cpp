/*
 * ModeModelMeshPolygon.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "MeshSelectionModePolygon.h"

#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/math/vec2.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../Animation/ModeModelAnimation.h"
#include "../../Skeleton/ModeModelSkeleton.h"
#include "MeshSelectionModeEdge.h"



MeshSelectionModePolygon::MeshSelectionModePolygon(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModePolygon::on_draw_win(MultiView::Window *win) {
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_MODEL_POLYGON))
		return;


	auto *m = data->edit_mesh;
	auto &p = m->polygon[multi_view->hover.index];
	VertexStagingBuffer vbs;
	p.add_to_vertex_buffer(m->show_vertices, vbs, 1);
	vbs.build(parent->vb_hover, 1);

	nix::set_offset(-2.0f);
	set_material_hover();
	nix::draw_triangles(parent->vb_hover);
	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
}



void MeshSelectionModePolygon::on_end() {
}



void MeshSelectionModePolygon::on_start() {
}



float poly_hover(ModelPolygon *pol, MultiView::Window *win, const vec2 &M, vector &tp, float &z, const Array<ModelVertex> &vertex) {
	// care for the sense of rotation?
	if (vector::dot(pol->temp_normal, win->get_direction()) > 0)
		return -1;

	// project all points
	Array<vector> p;
	for (int k=0;k<pol->side.num;k++) {
		vector pp = win->project(vertex[pol->side[k].vertex].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return -1;
		p.add(pp);
	}

	// test all sub-triangles
	if (pol->triangulation_dirty)
		pol->update_triangulation(vertex);
	for (int k=pol->side.num-3; k>=0; k--) {
		int a = pol->side[k].triangulation[0];
		int b = pol->side[k].triangulation[1];
		int c = pol->side[k].triangulation[2];
		auto fg = bary_centric(vector(M.x,M.y,0), p[a], p[b], p[c]);
		// cursor in triangle?
		if ((fg.x>0) and (fg.y>0) and (fg.x+fg.y<1)) {
			vector va = vertex[pol->side[a].vertex].pos;
			vector vb = vertex[pol->side[b].vertex].pos;
			vector vc = vertex[pol->side[c].vertex].pos;
			tp = va+fg.x*(vb-va)+fg.y*(vc-va);
			z = win->project(tp).z;
			return 0;
		}
	}
	return -1;
}

float ModelPolygon::hover_distance(MultiView::Window *win, const vec2 &M, vector &tp, float &z) {
	auto *m = mode_model_mesh->data->edit_mesh; // surf->model;
	return poly_hover(this, win, M, tp, z, m->show_vertices);
}

bool ModelPolygon::in_rect(MultiView::Window *win, const rect &r) {
	// care for the sense of rotation?
	if (mode_model_mesh->select_cw)
		if (vector::dot(temp_normal, win->get_direction()) > 0)
			return false;

	auto *m = mode_model_mesh->data->edit_mesh; // surf->model;

	// all vertices within rectangle?
	for (int k=0; k<side.num; k++) {
		vector pp = win->project(m->show_vertices[side[k].vertex].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (r.inside({pp.x, pp.y}))
			return true;
	}
	return false;
}

bool ModelPolygon::overlap_rect(MultiView::Window *win, const rect &r) {
	return in_rect(win, r);
}

void MeshSelectionModePolygon::on_update_selection() {
	data->edit_mesh->selection_from_polygons();
}

void MeshSelectionModePolygon::on_view_stage_change() {
	for (auto &v: data->mesh->vertex)
		if (v.is_selected)
			v.view_stage = multi_view->view_stage;
		else
			v.view_stage = min(v.view_stage, multi_view->view_stage);
	for (auto &e: data->mesh->edge)
		if (e.is_selected)
			e.view_stage = multi_view->view_stage;
		else
			e.view_stage = min(e.view_stage, multi_view->view_stage);
}

void MeshSelectionModePolygon::update_multi_view() {
	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(MVD_MODEL_POLYGON,
			data->edit_mesh->polygon,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
	multi_view->add_data(MVD_MODEL_BALL,
			data->edit_mesh->ball,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);
	multi_view->add_data(MVD_MODEL_CYLINDER,
			data->edit_mesh->cylinder,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);
}

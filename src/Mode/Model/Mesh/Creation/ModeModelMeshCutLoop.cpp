/*
 * ModeModelMeshCutLoop.cpp
 *
 *  Created on: Feb 16, 2020
 *      Author: michi
 */

#include "ModeModelMeshCutLoop.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Data/Model/ModelSelection.h"
//#include "../../../../Action/Model/Mesh/Polygon/ActionModelSplitPolygon.h"
//#include "../../../../Action/Model/Mesh/Edge/ActionModelSplitEdge.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../MultiView/ColorScheme.h"
#include "../../../../lib/nix/nix.h"


ModeModelMeshCutLoop::ModeModelMeshCutLoop(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCutLoop", _parent) {}

void ModeModelMeshCutLoop::on_start() {
	valid_loop = false;

	message = _("click on an edge to cut");

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_edge);
	mode_model->allow_selection_modes(false);

	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);
}

int edge_get_opposite_polygon(ModelEdge &e, int poly, int &side) {
	if (e.polygon[0] == poly) {
		side = e.side[1];
		return e.polygon[1];
	}
	if (e.polygon[1] == poly) {
		side = e.side[0];
		return e.polygon[0];
	}
	return -1;
}


bool ModeModelMeshCutLoop::find_loop(int edge, const vector &v) {
	auto *m = data->edit_mesh;

	valid_loop = false;
	edges = {edge};
	edge_directions = {};
	int poly = m->edge[edge].polygon[0];
	int side = m->edge[edge].side[0];
	polygons = {poly};
	polygon_sides = {side};

	while (poly >= 0) {
		auto &p = m->polygon[poly];
		edge_directions.add(p.side[side].edge_direction);

		// only allow even sides
		if ((p.side.num % 2) == 1)
			return false;

		// opposite side
		side = (side + p.side.num/2) % p.side.num;
		polygon_sides.add(side);
		edge  = p.side[side].edge;

		// loop closed?
		if (edge == edges[0]) {
			valid_loop = true;
			return true;
		}

		edges.add(edge);

		// neighboring polygon
		poly = edge_get_opposite_polygon(m->edge[edge], poly, side);
		polygons.add(poly);
		polygon_sides.add(side);
	}
	return false;
}

Array<int> poly_subrange(ModelPolygon &p, int s0, int s1) {
	Array<int> v;
	if (s1 < s0)
		s1 += p.side.num;
	for (int i=s0; i<s1; i++)
		v.add(p.side[i % p.side.num].vertex);
	return v;
}

void ModeModelMeshCutLoop::on_left_button_up() {
	if (!valid_loop)
		return;

	auto *m = data->edit_mesh;
	int nv = m->vertex.num;

	data->begin_action_group("CutLoop");

	// add new points on edges
	for (vector &p: pos)
		data->addVertex(p, 0, NORMAL_MODE_ANGULAR);

	// save some data
	Array<ModelPolygon> temp_polys;
	ModelSelection sel;
	for (int i=0; i<edges.num; i++) {
		temp_polys.add(m->polygon[polygons[i]]);
		sel.polygon.add(polygons[i]);
	}

	// delete
	data->delete_selection(sel, false);


	// refill
	for (int i=0; i<edges.num; i++) {
		auto &p = temp_polys[i];
		auto v = poly_subrange(p, polygon_sides[i*2]+1, polygon_sides[i*2+1]+1);
		v.add(nv + (i + 1) % edges.num);
		v.add(nv + i);
		data->addPolygon(v, p.material);

		v = poly_subrange(p, polygon_sides[i*2+1]+1, polygon_sides[i*2]+1);
		v.add(nv + i);
		v.add(nv + (i + 1) % edges.num);
		data->addPolygon(v, p.material);
	}

	data->end_action_group();

	//abort();
}

void ModeModelMeshCutLoop::calculate_edge_positions(float f) {
	auto *m = data->edit_mesh;
	pos.clear();
	foreachi (int e, edges, i)
		pos.add((1 - f) * m->vertex[m->edge[e].vertex[edge_directions[i]]].pos + f * m->vertex[m->edge[e].vertex[1 - edge_directions[i]]].pos);
}

void ModeModelMeshCutLoop::on_draw_win(MultiView::Window *win) {
	parent->on_draw_win(win);

	if (multi_view->hover.type != MVD_MODEL_EDGE)
		return;

	int edge0 = multi_view->hover.index;
	vector pos0 = multi_view->hover.point;
	if (edge0 < 0)
		return;

	if (!find_loop(edge0, pos0))
		return;

	auto *m = data->edit_mesh;
	float f = _vec_factor_between_(pos0, m->vertex[m->edge[edge0].vertex[0]].pos, m->vertex[m->edge[edge0].vertex[1]].pos);


	calculate_edge_positions(f);

	nix::SetZ(false, false);
	nix::SetWire(false);
	nix::SetColor(scheme.CREATION_LINE);
	set_wide_lines(scheme.LINE_WIDTH_MEDIUM);
	for (int k=0; k<edges.num; k++)
		nix::DrawLine3D(pos[k], pos[(k + 1) % edges.num]);
	nix::SetZ(true, true);
}

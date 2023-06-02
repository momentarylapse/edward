/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeSurface.h"
#include "MeshSelectionModePolygon.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../lib/nix/nix.h"


MeshSelectionModeSurface::MeshSelectionModeSurface(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{}


void expand_sel_to_surfaces(ModelMesh *m) {
	while (true) {
		bool changed = false;
		for (auto &e: m->edge)
			if (m->vertex[e.vertex[0]].is_selected != m->vertex[e.vertex[1]].is_selected) {
				m->vertex[e.vertex[0]].is_selected = true;
				m->vertex[e.vertex[1]].is_selected = true;
				changed = true;
			}
		if (!changed)
			break;
	}
	m->selection_from_vertices();
}

void MeshSelectionModeSurface::on_update_selection() {
	data->edit_mesh->selection_from_polygons();
	expand_sel_to_surfaces(data->edit_mesh);
}

void MeshSelectionModeSurface::on_view_stage_change() {
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

void MeshSelectionModeSurface::update_multi_view() {
	multi_view->clear_data(data);
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

void MeshSelectionModeSurface::on_draw_win(MultiView::Window *win) {
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_MODEL_POLYGON))
		return;

	VertexStagingBuffer vbs;
	auto m = data->edit_mesh;
	auto &p = m->polygon[multi_view->hover.index];
	p.add_to_vertex_buffer(m->show_vertices, vbs, 1);
	vbs.build(parent->vb_hover, 1);

	nix::set_offset(-2.0f);
	set_material_hover();
	nix::draw_triangles(parent->vb_hover);
	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
}



void MeshSelectionModeSurface::on_end() {
}



void MeshSelectionModeSurface::on_start() {
	expand_sel_to_surfaces(data->edit_mesh);
}



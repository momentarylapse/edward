/*
 * ModeModelMeshSurface.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "MeshSelectionModeSurface.h"

#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../Data/Model/ModelSurface.h"
#include "../ModeModelMesh.h"
#include "MeshSelectionModePolygon.h"
#include "../../ModeModel.h"


MeshSelectionModeSurface::MeshSelectionModeSurface(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{}


void expand_sel_to_surfaces(DataModel *m) {
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
	m->selectionFromVertices();
}

void MeshSelectionModeSurface::update_selection() {
	data->selectionFromPolygons();
	expand_sel_to_surfaces(data);
	//data->selectionFromSurfaces();
}

void MeshSelectionModeSurface::update_multi_view() {
	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(	MVD_MODEL_POLYGON,
			data->polygon,
			NULL,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

void MeshSelectionModeSurface::on_draw_win(MultiView::Window *win) {
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_MODEL_POLYGON))
		return;

	parent->vb_hover->clear();

	auto &p = data->polygon[multi_view->hover.index];
	p.addToVertexBuffer(data->show_vertices, parent->vb_hover, 1);
	/*ModelSurface &s = data->surface[multi_view->hover.index];
	for (ModelPolygon &p: s.polygon)
		p.addToVertexBuffer(data->show_vertices, parent->vb_hover, 1);
*/

	nix::SetWire(false);
	nix::SetOffset(1.0f);
	mode_model->set_material_hover();
	nix::Draw3D(parent->vb_hover);
	nix::SetMaterial(White,White,Black,0,Black);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetOffset(0);
	nix::SetWire(multi_view->wire_mode);
}



void MeshSelectionModeSurface::on_end() {
}

bool ModelSurface::hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data) {
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].hover(win, m, tp, z, user_data))
			return true;
	return false;
}

bool ModelSurface::inRect(MultiView::Window *win, rect &r, void *user_data) {
	for (int i=0;i<polygon.num;i++)
		if (polygon[i].inRect(win, r, user_data))
			return true;
	return false;
}



void MeshSelectionModeSurface::on_start() {
	expand_sel_to_surfaces(data);
}



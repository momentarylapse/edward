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
#include "../../../../lib/nix/nix.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../Animation/ModeModelAnimation.h"
#include "../../Skeleton/ModeModelSkeleton.h"
#include "MeshSelectionModeEdge.h"



MeshSelectionModePolygon::MeshSelectionModePolygon(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModePolygon::on_draw_win(MultiView::Window *win)
{
	if ((multi_view->hover.index < 0) or (multi_view->hover.type != MVD_MODEL_POLYGON))
		return;

	parent->vb_hover->clear();


	auto &p = data->polygon[multi_view->hover.index];
	p.addToVertexBuffer(data->show_vertices, parent->vb_hover, 1);

	nix::SetWire(false);
	nix::SetOffset(-1.0f);
	mode_model->set_material_hover();
	nix::Draw3D(parent->vb_hover);
	nix::SetMaterial(White,White,Black,0,Black);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetOffset(0);
	nix::SetWire(multi_view->wire_mode);
}



void MeshSelectionModePolygon::on_end()
{
}



void MeshSelectionModePolygon::on_start()
{
}


bool poly_hover(ModelPolygon *pol, MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data, const Array<ModelVertex> &vertex)
{
	// care for the sense of rotation?
	if (pol->temp_normal * win->getDirection() > 0)
		return false;

	// project all points
	Array<vector> p;
	for (int k=0;k<pol->side.num;k++){
		vector pp = win->project(vertex[pol->side[k].vertex].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		p.add(pp);
	}

	// test all sub-triangles
	if (pol->triangulation_dirty)
		pol->updateTriangulation(vertex);
	for (int k=pol->side.num-3; k>=0; k--){
		int a = pol->side[k].triangulation[0];
		int b = pol->side[k].triangulation[1];
		int c = pol->side[k].triangulation[2];
		float f,g;
		GetBaryCentric(M, p[a], p[b], p[c], f, g);
		// cursor in triangle?
		if ((f>0)&&(g>0)&&(f+g<1)){
			vector va = vertex[pol->side[a].vertex].pos;
			vector vb = vertex[pol->side[b].vertex].pos;
			vector vc = vertex[pol->side[c].vertex].pos;
			tp = va+f*(vb-va)+g*(vc-va);
			z = win->project(tp).z;
			return true;
		}
	}
	return false;
}
bool ModelPolygon::hover(MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data)
{
	DataModel *m = mode_model_mesh->data; // surf->model;
	return poly_hover(this, win, M, tp, z, user_data, m->show_vertices);
}

inline bool in_irect(const vector &p, rect &r)
{
	return ((p.x > r.x1) and (p.x < r.x2) and (p.y > r.y1) and (p.y < r.y2));
}

bool ModelPolygon::inRect(MultiView::Window *win, rect &r, void *user_data)
{
	// care for the sense of rotation?
	if (mode_model_mesh->select_cw)
		if (temp_normal * win->getDirection() > 0)
			return false;

	DataModel *m = mode_model_mesh->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<side.num;k++){
		vector pp = win->project(m->show_vertices[side[k].vertex].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (in_irect(pp, r))
			return true;
	}
	return false;
}


void MeshSelectionModePolygon::update_selection() {
	data->selectionFromPolygons();
}

void MeshSelectionModePolygon::update_multi_view() {
	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(	MVD_MODEL_POLYGON,
			data->polygon,
			NULL,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

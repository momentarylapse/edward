/*
 * ModeModelMeshPolygon.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "MeshSelectionModePolygon.h"

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "../ModeModel.h"
#include "../Animation/ModeModelAnimation.h"
#include "../Skeleton/ModeModelSkeleton.h"

#include <GL/gl.h>
#include "MeshSelectionModeEdge.h"


MeshSelectionModePolygon::MeshSelectionModePolygon(ModeModelMesh *_parent) :
	MeshSelectionMode(_parent)
{
}

void MeshSelectionModePolygon::onDrawWin(MultiView::Window *win)
{
	if ((multi_view->hover.index < 0) || (multi_view->hover.type != MVD_MODEL_POLYGON))
		return;

	parent->vb_hover->clear();


	ModelPolygon &p = data->surface[multi_view->hover.set].polygon[multi_view->hover.index];
	p.AddToVertexBuffer(data->vertex, parent->vb_hover, 1);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	mode_model->setMaterialMouseOver();
	parent->vb_hover->draw();
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);
}



void MeshSelectionModePolygon::onEnd()
{
}



void MeshSelectionModePolygon::onStart()
{
	multi_view->setAllowRect(true);
}



bool ModelPolygon::hover(MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data)
{
	// care for the sense of rotation?
	if (temp_normal * win->getDirection() > 0)
		return false;

	DataModel *m = mode_model_mesh->data; // surf->model;

	// project all points
	Array<vector> p;
	for (int k=0;k<side.num;k++){
		vector pp = win->project(m->vertex[side[k].vertex].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		p.add(pp);
	}

	// test all sub-triangles
	if (triangulation_dirty)
		UpdateTriangulation(m->vertex);
	for (int k=side.num-3; k>=0; k--){
		int a = side[k].triangulation[0];
		int b = side[k].triangulation[1];
		int c = side[k].triangulation[2];
		float f,g;
		GetBaryCentric(M, p[a], p[b], p[c], f, g);
		// cursor in triangle?
		if ((f>0)&&(g>0)&&(f+g<1)){
			vector va = m->vertex[side[a].vertex].pos;
			vector vb = m->vertex[side[b].vertex].pos;
			vector vc = m->vertex[side[c].vertex].pos;
			tp = va+f*(vb-va)+g*(vc-va);
			z = win->project(tp).z;
			return true;
		}
	}
	return false;
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
		vector pp = win->project(m->vertex[side[k].vertex].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (in_irect(pp, r))
			return true;
	}
	return false;
}


void MeshSelectionModePolygon::updateSelection()
{
	data->SelectionFromPolygons();
}

void MeshSelectionModePolygon::updateMultiView()
{
	multi_view->clearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	foreach(ModelSurface &s, data->surface)
	multi_view->addData(	MVD_MODEL_POLYGON,
			s.polygon,
			&s,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

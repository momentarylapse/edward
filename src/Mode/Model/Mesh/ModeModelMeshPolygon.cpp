/*
 * ModeModelMeshPolygon.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMeshPolygon.h"
#include "ModeModelMeshEdge.h"
#include "ModeModelMesh.h"
#include "../Animation/ModeModelAnimation.h"

#include <GL/gl.h>


ModeModelMeshPolygon *mode_model_mesh_polygon = NULL;

ModeModelMeshPolygon::ModeModelMeshPolygon(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshSkin", _parent, ed->multi_view_3d, "menu_model")
{
	// vertex buffers
	VBMarked = new NixVertexBuffer(1);
	VBModel = new NixVertexBuffer(1);
	VBModel2 = NULL;
	VBModel3 = NULL;
	VBModel4 = NULL;
	VBMouseOver = new NixVertexBuffer(1);
	VBCreation = new NixVertexBuffer(1);

	SelectCW = false;
}

ModeModelMeshPolygon::~ModeModelMeshPolygon()
{
}

void ModeModelMeshPolygon::DrawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	msg_db_f("ModelSkin.DrawPolys",2);

	if (multi_view->wire_mode){
		mode_model_mesh_edge->DrawEdges(win, vertex, false);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial &m, data->material, mi){
		NixVertexBuffer **vb = &VBModel;
		int num_tex = min(m.num_textures, 4);
		if (num_tex == 2)
			vb = &VBModel2;
		else if (num_tex == 3)
			vb = &VBModel3;
		else if (num_tex == 4)
			vb = &VBModel4;
		if (!*vb)
			*vb = new NixVertexBuffer(num_tex);

		(*vb)->clear();

		foreach(ModelSurface &surf, data->surface){
			if (!surf.is_visible)
				continue;
			foreach(ModelPolygon &t, surf.polygon)
				if ((t.view_stage >= multi_view->view_stage) && (t.material == mi))
					t.AddToVertexBuffer(vertex, *vb, m.num_textures);
		}

		// draw
		m.ApplyForRendering();
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		NixDraw3D(*vb);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		NixSetShader(NULL);
		NixSetTexture(NULL);
	}

	mode_model_mesh_edge->DrawEdges(win, vertex, true);
}

void ModeModelMeshPolygon::onCommand(const string & id)
{
}

void ModeModelMeshPolygon::onUpdateMenu()
{
}

void ModeModelMeshPolygon::FillSelectionBuffers(Array<ModelVertex> &vertex)
{
	msg_db_f("SkinFillSelBuf", 4);
	VBMarked->clear();
	VBMouseOver->clear();

	// create selection buffers
	msg_db_m("a",4);
	ModelPolygon *mmo = NULL;
	if ((multi_view->hover.index >= 0) && (multi_view->hover.set < data->surface.num) && (multi_view->hover.type == MVDModelPolygon))
		mmo = &data->surface[multi_view->hover.set].polygon[multi_view->hover.index];
	foreachi(ModelSurface &s, data->surface, si){
		bool s_mo = false;
		if ((multi_view->hover.index >= 0) && (multi_view->hover.type == MVDModelSurface))
			s_mo = (multi_view->hover.index == si);
		foreach(ModelPolygon &t, s.polygon)
			/*if (t.view_stage >= ViewStage)*/{
			if (t.is_selected)
				t.AddToVertexBuffer(vertex, VBMarked, 1);
			if ((&t == mmo) || (s_mo))
				t.AddToVertexBuffer(vertex, VBMouseOver, 1);
		}
	}
}

void ModeModelMeshPolygon::SetMaterialMarked()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void ModeModelMeshPolygon::SetMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void ModeModelMeshPolygon::SetMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0.3f,1,0.3f),Black,0,color(1,0.1f,0.4f,0.1f));
}

void ModeModelMeshPolygon::DrawSelection(MultiView::Window *win)
{
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	SetMaterialMarked();
	NixDraw3D(VBMarked);
	SetMaterialMouseOver();
	NixDraw3D(VBMouseOver);
	SetMaterialCreation();
	NixDraw3D(VBCreation);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);
}

void ModeModelMeshPolygon::onDrawWin(MultiView::Window *win)
{
	msg_db_f("skin.DrawWin",4);

	DrawPolygons(win, data->vertex);
	DrawSelection(win);
}



void ModeModelMeshPolygon::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
}



void ModeModelMeshPolygon::onStart()
{
	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	mode_model_mesh->applyMouseFunction(multi_view);
	multi_view->SetAllowRect(true);
	onUpdate(data, "");
}



bool ModelPolygon::hover(MultiView::Window *win, vector &M, vector &tp, float &z, void *user_data)
{
	// care for the sense of rotation?
	if (temp_normal * win->getDirection() > 0)
		return false;

	DataModel *m = mode_model_mesh_polygon->data; // surf->model;

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
	if (mode_model_mesh_polygon->SelectCW)
		if (temp_normal * win->getDirection() > 0)
			return false;

	DataModel *m = mode_model_mesh_polygon->data; // surf->model;

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


void ModeModelMeshPolygon::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		foreach(ModelSurface &s, data->surface)
		multi_view->AddData(	MVDModelPolygon,
				s.polygon,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o == multi_view){
		data->SelectionFromPolygons();
	}
	FillSelectionBuffers(data->vertex);
}



void ModeModelMeshPolygon::onDraw()
{
	FillSelectionBuffers(data->vertex);
}

void ModeModelMeshPolygon::ToggleSelectCW()
{
	SelectCW = !SelectCW;
	ed->updateMenu();
}





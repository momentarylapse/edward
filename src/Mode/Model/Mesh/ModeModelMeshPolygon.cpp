/*
 * ModeModelMeshPolygon.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMeshPolygon.h"
#include "ModeModelMesh.h"
#include "../Animation/ModeModelAnimation.h"



ModeModelMeshPolygon *mode_model_mesh_polygon = NULL;

ModeModelMeshPolygon::ModeModelMeshPolygon(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshSkin", _parent, ed->multi_view_3d, "menu_model")
{
	// vertex buffers
	VBMarked = NixCreateVB(65536, 1);
	VBModel = NixCreateVB(65536, 1);
	VBModel2 = -1;
	VBModel3 = -1;
	VBModel4 = -1;
	VBMouseOver = NixCreateVB(8192, 1);
	VBCreation = NixCreateVB(8192, 1);

	SelectCW = false;
}

ModeModelMeshPolygon::~ModeModelMeshPolygon()
{
}

void ModeModelMeshPolygon::DrawPolygons(Array<ModelVertex> &vertex)
{
	msg_db_f("ModelSkin.DrawPolys",2);

	if (multi_view->wire_mode){
		NixSetWire(false);
		NixEnableLighting(false);
		vector dir = multi_view->cam.ang.ang2dir();
		foreach(ModelSurface &s, data->Surface){
			foreach(ModelEdge &e, s.Edge){
				if (min(vertex[e.Vertex[0]].view_stage, vertex[e.Vertex[1]].view_stage) < multi_view->view_stage)
					continue;
				if (e.is_selected){
					NixSetColor(Red);
				}else{
					float f = 0.7f - (s.Polygon[e.Polygon[0]].TempNormal * dir) * 0.3f;
					NixSetColor(color(1, f, f, f));
				}
				NixDrawLine3D(vertex[e.Vertex[0]].pos, vertex[e.Vertex[1]].pos);
			}
		}
		NixSetColor(White);
		NixSetWire(true);
		NixEnableLighting(multi_view->light_enabled);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial &m, data->Material, mi){
		int *vb = &VBModel;
		int num_tex = min(m.NumTextures, 4);
		if (num_tex == 2)
			vb = &VBModel2;
		else if (num_tex == 3)
			vb = &VBModel3;
		else if (num_tex == 4)
			vb = &VBModel4;
		if (*vb < 0)
			*vb = NixCreateVB(65536, num_tex);

		NixVBClear(*vb);

		foreach(ModelSurface &surf, data->Surface)
			foreach(ModelPolygon &t, surf.Polygon)
				if ((t.view_stage >= multi_view->view_stage) && (t.Material == mi))
					t.AddToVertexBuffer(vertex, *vb, m.NumTextures);

		// draw
		m.ApplyForRendering();
		NixDraw3D(*vb);
		NixSetTexture(-1);
	}
}

void ModeModelMeshPolygon::OnCommand(const string & id)
{
}

void ModeModelMeshPolygon::OnUpdateMenu()
{
}

void ModeModelMeshPolygon::FillSelectionBuffers(Array<ModelVertex> &vertex)
{
	msg_db_f("SkinFillSelBuf", 4);
	NixVBClear(VBMarked);
	NixVBClear(VBMouseOver);

	// create selection buffers
	msg_db_m("a",4);
	ModelPolygon *mmo = NULL;
	if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverSet < data->Surface.num) && (multi_view->MouseOverType == MVDModelPolygon))
		mmo = &data->Surface[multi_view->MouseOverSet].Polygon[multi_view->MouseOver];
	foreachi(ModelSurface &s, data->Surface, si){
		bool s_mo = false;
		if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverType == MVDModelSurface))
			s_mo = (multi_view->MouseOver == si);
		foreach(ModelPolygon &t, s.Polygon)
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

void ModeModelMeshPolygon::OnDrawWin(MultiViewWindow *win)
{
	msg_db_f("skin.DrawWin",4);

	/*if (Detail==DetailPhysical){
		SetMaterialPhysical();
		NixDraw3D(-1,VBModel,m_id);
	}else*/
		DrawPolygons(data->Vertex);
	NixSetShader(-1);
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);
	msg_db_m("----a",4);

	SetMaterialMarked();
	NixDraw3D(VBMarked);
	SetMaterialMouseOver();
	NixDraw3D(VBMouseOver);
	SetMaterialCreation();
	NixDraw3D(VBCreation);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
}



void ModeModelMeshPolygon::OnEnd()
{
	multi_view->ResetData(NULL);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshPolygon::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->MVRectable = true;
	OnUpdate(data);
}



bool PolygonIsMouseOver(int index, void *user_data, MultiViewWindow *win, vector &tp)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelPolygon *t = &surf->Polygon[index];

	// care for the sense of rotation?
	if (t->TempNormal * win->GetDirection() > 0)
		return false;

	DataModel *m = mode_model_mesh_polygon->data; // surf->model;

	// project all points
	Array<vector> p;
	for (int k=0;k<t->Side.num;k++){
		vector pp = win->Project(m->Vertex[t->Side[k].Vertex].pos);
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		p.add(pp);
	}

	// test all sub-triangles
	if (t->TriangulationDirty)
		t->UpdateTriangulation(m->Vertex);
	vector M = win->multi_view->m;
	for (int k=t->Side.num-3; k>=0; k--){
		int a = t->Side[k].Triangulation[0];
		int b = t->Side[k].Triangulation[1];
		int c = t->Side[k].Triangulation[2];
		float f,g;
		GetBaryCentric(M, p[a], p[b], p[c], f, g);
		// cursor in triangle?
		if ((f>0)&&(g>0)&&(f+g<1)){
			vector va = m->Vertex[t->Side[a].Vertex].pos;
			vector vb = m->Vertex[t->Side[b].Vertex].pos;
			vector vc = m->Vertex[t->Side[c].Vertex].pos;
			tp = va+f*(vb-va)+g*(vc-va);
			return true;
		}
	}
	return false;
}

inline bool in_irect(const vector &p, rect *r)
{
	return ((p.x > r->x1) and (p.x < r->x2) and (p.y > r->y1) and (p.y < r->y2));
}

bool PolygonInRect(int index, void *user_data, MultiViewWindow *win, rect *r)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelPolygon *t = &surf->Polygon[index];

	// care for the sense of rotation?
	if (mode_model_mesh_polygon->SelectCW)
		if (t->TempNormal * win->GetDirection() > 0)
			return false;

	DataModel *m = mode_model_mesh_polygon->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<t->Side.num;k++){
		vector pp = win->Project(m->Vertex[t->Side[k].Vertex].pos); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!in_irect(pp, r))
			return false;
	}
	return true;
}


void ModeModelMeshPolygon::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		foreach(ModelSurface &s, data->Surface)
		multi_view->SetData(	MVDModelPolygon,
				s.Polygon,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&PolygonIsMouseOver, &PolygonInRect);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromPolygons();
	}
	FillSelectionBuffers(data->Vertex);
}



void ModeModelMeshPolygon::OnDraw()
{
	FillSelectionBuffers(data->Vertex);
}

void ModeModelMeshPolygon::ToggleSelectCW()
{
	SelectCW = !SelectCW;
	ed->UpdateMenu();
}





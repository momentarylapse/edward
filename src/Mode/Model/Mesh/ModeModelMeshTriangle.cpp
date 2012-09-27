/*
 * ModeModelMeshSkin.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMeshTriangle.h"
#include "ModeModelMesh.h"
#include "../Animation/ModeModelAnimation.h"



ModeModelMeshTriangle *mode_model_mesh_triangle = NULL;

ModeModelMeshTriangle::ModeModelMeshTriangle(Mode *_parent, DataModel *_data) :
	Mode("ModelMeshSkin", _parent, _data, ed->multi_view_3d, "menu_model")
{
	data = _data;

	// vertex buffers
	VBMarked = NixCreateVB(65536);
	VBModel = NixCreateVB(65536);
	VBModel2 = -1;
	VBModel3 = -1;
	VBModel4 = -1;
	VBMouseOver = NixCreateVB(1024);
	VBCreation = NixCreateVB(1024);

	SelectCW = false;
}

ModeModelMeshTriangle::~ModeModelMeshTriangle()
{
}

//#define GetVertex(v)	data->Vertex[v].pos
inline vector GetVertex(int v)
{
	DataModel *m = mode_model_mesh_triangle->data;
	if (mode_model_animation->IsAncestorOf(ed->cur_mode))
		return m->Vertex[v].AnimatedPos;
	return m->Vertex[v].pos;
}

inline void add_tria(int vb, const DataModel *data, const ModelPolygon &t)
{
	vector tv1 = t.Side[0].SkinVertex[0];
	for (int k=2;k<t.Side.num;k++){
		vector tv2 = t.Side[k-1].SkinVertex[0];
		vector tv3 = t.Side[k  ].SkinVertex[0];
		NixVBAddTria(	vb,
					GetVertex(t.Side[0  ].Vertex), t.Side[0  ].Normal, tv1.x, tv1.y,
					GetVertex(t.Side[k-1].Vertex), t.Side[k-1].Normal, tv2.x, tv2.y,
					GetVertex(t.Side[k  ].Vertex), t.Side[k  ].Normal, tv3.x, tv3.y);
	}
}

void ModeModelMeshTriangle::DrawTrias()
{
	msg_db_r("ModelSkin.DrawTrias",2);

	if (multi_view->wire_mode){
		NixSetWire(false);
		NixEnableLighting(false);
		NixSetColor(White);
		foreach(ModelSurface &s, data->Surface){
			foreach(ModelEdge &e, s.Edge)
				NixDrawLine3D(GetVertex(e.Vertex[0]), GetVertex(e.Vertex[1]));
		}
		NixSetWire(true);
		NixEnableLighting(multi_view->light_enabled);
		msg_db_l(2);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial &m, data->Material, mi){
		m.ApplyForRendering();

		// single texture
		if (m.NumTextures == 1){
			NixVBClear(VBModel);

			foreach(ModelSurface &surf, data->Surface)
				foreach(ModelPolygon &t, surf.Polygon)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi))
						add_tria(VBModel, data, t);

			// draw
			NixSetTexture(m.Texture[0]);
			NixDraw3D(VBModel); // TODO:  alle Texturen, nicht nur die erste....
			NixSetTexture(-1);

		// multi texture
		}else{
			int num_tex = m.NumTextures;
			if (num_tex > 4)
				num_tex = 4;
			int *vb = &VBModel2;
			if (num_tex == 3)
				vb = &VBModel3;
			if (num_tex == 4)
				vb = &VBModel4;
			if (*vb < 0)
				*vb = NixCreateVBM(65536, num_tex);

			NixVBClear(*vb);

			foreach(ModelSurface &surf, data->Surface)
				foreach(ModelPolygon &t, surf.Polygon)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi)){
						for (int k=2;k<t.Side.num;k++){
							float t1[8], t2[8], t3[8];
							for (int tl=0;tl<num_tex;tl++){
								t1[tl*2  ] = t.Side[0  ].SkinVertex[tl].x;
								t1[tl*2+1] = t.Side[0  ].SkinVertex[tl].y;
								t2[tl*2  ] = t.Side[k-1].SkinVertex[tl].x;
								t2[tl*2+1] = t.Side[k-1].SkinVertex[tl].y;
								t3[tl*2  ] = t.Side[k  ].SkinVertex[tl].x;
								t3[tl*2+1] = t.Side[k  ].SkinVertex[tl].y;
							}
							NixVBAddTriaM(	*vb,
											GetVertex(t.Side[0  ].Vertex), t.Side[0  ].Normal, t1,
											GetVertex(t.Side[k-1].Vertex), t.Side[k-1].Normal, t2,
											GetVertex(t.Side[k  ].Vertex), t.Side[k  ].Normal, t3);
						}
					}

			// draw
			NixSetTextures(m.Texture, num_tex);
			NixDraw3DM(*vb);
			NixSetTexture(-1);
		}
	}
	msg_db_l(2);
}

void ModeModelMeshTriangle::OnCommand(const string & id)
{
}

void ModeModelMeshTriangle::OnUpdateMenu()
{
}

void ModeModelMeshTriangle::FillSelectionBuffers()
{
	msg_db_r("SkinFillSelBuf", 4);
	NixVBClear(VBMarked);
	NixVBClear(VBMouseOver);

	// create selection buffers
	msg_db_m("a",4);
	/*if ((EditMode == EditModeTriangle) || (EditMode == EditModeVertex) || (EditMode == EditModeEdge))*/{
		ModelPolygon *mmo = NULL;
		if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverSet < data->Surface.num) && (multi_view->MouseOverType == MVDModelTriangle))
			mmo = &data->Surface[multi_view->MouseOverSet].Polygon[multi_view->MouseOver];
		foreachi(ModelSurface &s, data->Surface, si){
			bool s_mo = false;
			if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverType == MVDModelSurface))
				s_mo = (multi_view->MouseOver == si);
			foreach(ModelPolygon &t, s.Polygon)
				/*if (t.view_stage >= ViewStage)*/{
				if (t.is_selected)
					add_tria(VBMarked, data, t);
				if ((&t == mmo) || (s_mo))
					add_tria(VBMouseOver, data, t);
			}
		}
	}/*else if (EditMode == EditModeSurface){
		ModeModelSurface *mmo = NULL;
		if ((MouseOver >= 0) && (MouseOverType == MVDModelSurface))
			mmo = &Surface[MouseOver];
		foreach(Surface, surf)
			if (surf->ViewStage >= ViewStage){
				if (surf->IsSelected){
					foreach(surf->Triangle, t){
						NixVBAddTria(VBMarked,	GetVertex(t->Vertex[0]), t->Normal[0], 0, 0,
												GetVertex(t->Vertex[1]), t->Normal[1], 0, 0,
												GetVertex(t->Vertex[2]), t->Normal[2], 0, 0);
					}
				}
				if (surf == mmo){
					foreach(surf->Triangle, t){
						NixVBAddTria(VBMouseOver,	GetVertex(t->Vertex[0]), t->Normal[0], 0, 0,
													GetVertex(t->Vertex[1]), t->Normal[1], 0, 0,
													GetVertex(t->Vertex[2]), t->Normal[2], 0, 0);
					}
				}
			}
	}*/
	msg_db_l(4);
}

void SetMaterialMarked()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void SetMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void SetMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0.3f,1,0.3f),Black,0,color(1,0.1f,0.4f,0.1f));
}

void ModeModelMeshTriangle::OnDrawWin(int win, irect dest)
{
	msg_db_r("skin.DrawWin",4);

	/*if (Detail==DetailPhysical){
		SetMaterialPhysical();
		NixDraw3D(-1,VBModel,m_id);
	}else*/
		DrawTrias();
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

	msg_db_l(4);
}



void ModeModelMeshTriangle::OnEnd()
{
	multi_view->ResetData(NULL);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshTriangle::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->MVRectable = true;
	OnUpdate(data);
}



bool TriangleIsMouseOver(int index, void *user_data, int win, vector &tp)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelPolygon *t = &surf->Polygon[index];

	// care for the sense of rotation?
	if (t->TempNormal * ed->multi_view_3d->GetDirection(win) > 0)
		return false;

	DataModel *m = mode_model_mesh_triangle->data; // surf->model;

	// project all points
	Array<vector> p;
	for (int k=0;k<t->Side.num;k++){
		vector pp = ed->multi_view_3d->VecProject(m->Vertex[t->Side[k].Vertex].pos, win); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		p.add(pp);
	}

	// test all sub-triangles
	Array<int> vi = t->Triangulate(m);
	vector M = vector(float(ed->multi_view_3d->mx), float(ed->multi_view_3d->my), 0);
	for (int i=0;i<vi.num/3;i++){
		float f,g;
		GetBaryCentric(M, p[vi[i*3]], p[vi[i*3+1]], p[vi[i*3+2]], f, g);
		// cursor in triangle?
		if ((f>0)&&(g>0)&&(f+g<1)){
			vector va = m->Vertex[t->Side[vi[i*3  ]].Vertex].pos;
			vector vb = m->Vertex[t->Side[vi[i*3+1]].Vertex].pos;
			vector vc = m->Vertex[t->Side[vi[i*3+2]].Vertex].pos;
			tp = va+f*(vb-va)+g*(vc-va);
			return true;
		}
	}
	return false;
}

inline bool in_irect(const vector &p, irect *r)
{
	return ((p.x > r->x1) and (p.x < r->x2) and (p.y > r->y1) and (p.y < r->y2));
}

bool TriangleInRect(int index, void *user_data, int win, irect *r)
{
	ModelSurface *surf = (ModelSurface*)user_data;
	ModelPolygon *t = &surf->Polygon[index];

	// care for the sense of rotation?
	if (mode_model_mesh_triangle->SelectCW)
		if (t->TempNormal * ed->multi_view_3d->GetDirection(win) > 0)
			return false;

	DataModel *m = mode_model_mesh_triangle->data; // surf->model;

	// all vertices within rectangle?
	for (int k=0;k<t->Side.num;k++){
		vector pp = ed->multi_view_3d->VecProject(m->Vertex[t->Side[k].Vertex].pos, win); // mmodel->GetVertex(ia)
		if ((pp.z <= 0) or (pp.z >= 1))
			return false;
		if (!in_irect(pp, r))
			return false;
	}
	return true;
}


void ModeModelMeshTriangle::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		foreach(ModelSurface &s, data->Surface)
		multi_view->SetData(	MVDModelTriangle,
				s.Polygon,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&TriangleIsMouseOver, &TriangleInRect);
	}else if (o->GetName() == "MultiView"){
		data->SelectionVerticesFromTriangles();
		data->SelectionSurfacesFromTriangles();
	}
	FillSelectionBuffers();
}



void ModeModelMeshTriangle::OnDraw()
{
	FillSelectionBuffers();
}

void ModeModelMeshTriangle::ToggleSelectCW()
{
	SelectCW = !SelectCW;
	ed->UpdateMenu();
}





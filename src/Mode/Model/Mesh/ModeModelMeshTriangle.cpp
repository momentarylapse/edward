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



ModeModelMeshTriangle *mode_model_mesh_triangle = NULL;

ModeModelMeshTriangle::ModeModelMeshTriangle(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshSkin";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");

	// vertex buffers
	VBMarked = NixCreateVB(65536);
	VBModel = NixCreateVB(65536);
	VBModel2 = -1;
	VBModel3 = -1;
	VBModel4 = -1;
	VBMouseOver = NixCreateVB(1024);
	VBCreation = NixCreateVB(1024);
}

ModeModelMeshTriangle::~ModeModelMeshTriangle()
{
}

#define GetVertex(v)	data->Vertex[v].pos

inline void add_tria(int vb, const DataModel *data, const ModeModelTriangle &t)
{
	vector tv1 = t.SkinVertex[0][0];
	vector tv2 = t.SkinVertex[0][1];
	vector tv3 = t.SkinVertex[0][2];
	NixVBAddTria(	vb,
					GetVertex(t.Vertex[0]), t.Normal[0], tv1.x, tv1.y,
					GetVertex(t.Vertex[1]), t.Normal[1], tv2.x, tv2.y,
					GetVertex(t.Vertex[2]), t.Normal[2], tv3.x, tv3.y);
}

void ModeModelMeshTriangle::DrawTrias()
{
	msg_db_r("ModelSkin.DrawTrias",2);

	// draw all materials separately
	foreachi(data->Material, m, mi){
		m.ApplyForRendering();

		// single texture
		if (m.NumTextures == 1){
			NixVBClear(VBModel);

			foreach(data->Surface, surf)
				foreach(surf.Triangle, t)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi))
						add_tria(VBModel, data, t);

			// draw
			NixDraw3D(m.Texture[0], VBModel, m_id); // TODO:  alle Texturen, nicht nur die erste....

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

			foreach(data->Surface, surf)
				foreach(surf.Triangle, t)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi)){
						float t1[8], t2[8], t3[8];
						for (int tl=0;tl<num_tex;tl++){
							t1[tl*2  ] = t.SkinVertex[tl][0].x;
							t1[tl*2+1] = t.SkinVertex[tl][0].y;
							t2[tl*2  ] = t.SkinVertex[tl][1].x;
							t2[tl*2+1] = t.SkinVertex[tl][1].y;
							t3[tl*2  ] = t.SkinVertex[tl][2].x;
							t3[tl*2+1] = t.SkinVertex[tl][2].y;
						}
						NixVBAddTriaM(	*vb,
										GetVertex(t.Vertex[0]), t.Normal[0], t1,
										GetVertex(t.Vertex[1]), t.Normal[1], t2,
										GetVertex(t.Vertex[2]), t.Normal[2], t3);
					}

			// draw
			NixDraw3DM(m.Texture, *vb, m_id);
		}
	}
	msg_db_l(2);
}

void ModeModelMeshTriangle::FillSelectionBuffers()
{
	msg_db_r("SkinFillSelBuf", 4);
	NixVBClear(VBMarked);
	NixVBClear(VBMouseOver);

	// create selection buffers
	msg_db_m("a",4);
	/*if ((EditMode == EditModeTriangle) || (EditMode == EditModeVertex) || (EditMode == EditModeEdge))*/{
		ModeModelTriangle *mmo = NULL;
		if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverSet < data->Surface.num) && (multi_view->MouseOverType == MVDModelTriangle))
			mmo = &data->Surface[multi_view->MouseOverSet].Triangle[multi_view->MouseOver];
		foreachi(data->Surface, s, si){
			bool s_mo = false;
			if ((multi_view->MouseOver >= 0) && (multi_view->MouseOverType == MVDModelSurface))
				s_mo = (multi_view->MouseOver == si);
			foreach(s.Triangle, t)
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

void ModeModelMeshTriangle::DrawWin(int win, irect dest)
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
	NixDraw3D(-1,VBMarked,m_id);
	SetMaterialMouseOver();
	NixDraw3D(-1,VBMouseOver,m_id);
	SetMaterialCreation();
	NixDraw3D(-1,VBCreation,m_id);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);

	msg_db_l(4);
}



void ModeModelMeshTriangle::OnMiddleButtonDown()
{
}



void ModeModelMeshTriangle::OnUpdateMenu()
{
}



void ModeModelMeshTriangle::End()
{
}



void ModeModelMeshTriangle::OnKeyDown()
{
}



void ModeModelMeshTriangle::Start()
{
	OnUpdate(data);
}



bool TriangleIsMouseOver(int index, void *user_data, int win, vector &tp)
{
	ModeModelSurface *surf = (ModeModelSurface*)user_data;
	ModeModelTriangle *t = &surf->Triangle[index];
	vector M = vector(float(ed->multi_view_3d->mx), float(ed->multi_view_3d->my), 0);
	int a = t->Vertex[0];
	int b = t->Vertex[1];
	int c = t->Vertex[2];
	vector va = mode_model_mesh_triangle->data->Vertex[a].pos;//mmodel->GetVertex(a);
	vector vb = mode_model_mesh_triangle->data->Vertex[b].pos;//mmodel->GetVertex(b);
	vector vc = mode_model_mesh_triangle->data->Vertex[c].pos;//mmodel->GetVertex(c);
	vector pa = ed->multi_view_3d->VecProject(va, win);
	vector pb = ed->multi_view_3d->VecProject(vb, win);
	vector pc = ed->multi_view_3d->VecProject(vc, win);
	if ((pa.z>0)&&(pb.z>0)&&(pc.z>0)&&(pa.z<1)&&(pb.z<1)&&(pc.z<1)){
		float f,g;
		GetBaryCentric(M,pa,pb,pc,f,g);
		// cursor in triangle?
		if ((f>0)&&(g>0)&&(f+g<1)){
			// rechts- oder links-herum?
			float wba=(float)atan2(pb.x-pa.x,pb.y-pa.y);
			float wca=(float)atan2(pc.x-pa.x,pc.y-pa.y);
			if (wba<0)	wba+=2*pi;
			if (wca<0)	wca+=2*pi;
			float dw=wca-wba;
			if (dw<0)	dw+=2*pi;
			if (dw>pi){
				// Mauspunkt mit Tiefe z
				tp=va+f*(vb-va)+g*(vc-va);
				return true;
			}
		}
	}
	return false;
}

bool TriangleInRect(int index, void *user_data, int win, irect *r)
{
	ModeModelSurface *surf = (ModeModelSurface*)user_data;
	ModeModelTriangle *t = &surf->Triangle[index];
	int ia=t->Vertex[0];
	int ib=t->Vertex[1];
	int ic=t->Vertex[2];
	vector A,B,C;
	A = ed->multi_view_3d->VecProject(mode_model_mesh_triangle->data->Vertex[ia].pos, win); // mmodel->GetVertex(ia)
	B = ed->multi_view_3d->VecProject(mode_model_mesh_triangle->data->Vertex[ib].pos, win);
	C = ed->multi_view_3d->VecProject(mode_model_mesh_triangle->data->Vertex[ic].pos, win);
	// all vertices within rectangle?
	if ((A.z>0)&&(B.z>0)&&(C.z>0))
		if ((A.x>r->x1)&&(A.x<r->x2)&&(A.y>r->y1)&&(A.y<r->y2))
			if ((B.x>r->x1)&&(B.x<r->x2)&&(B.y>r->y1)&&(B.y<r->y2))
				if ((C.x>r->x1)&&(C.x<r->x2)&&(C.y>r->y1)&&(C.y<r->y2)){
					// care for the sense of rotation?
					if (false){//SelectCW){
						float wba=(float)atan2(B.x-A.x,B.y-A.y);
						float wca=(float)atan2(C.x-A.x,C.y-A.y);
						if (wba<0)	wba+=2*pi;
						if (wca<0)	wca+=2*pi;
						float dw=wca-wba;
						if (dw<0)	dw+=2*pi;
						if (dw>pi)
							return true;
					}else
						return true;
				}
	return false;
}


void ModeModelMeshTriangle::OnUpdate(Observable *o)
{
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){
		multi_view->ResetData(data);
		mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		foreach(data->Surface, s)
		multi_view->SetData(	MVDModelTriangle,
				s.Triangle,
				&s,
				MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				&TriangleIsMouseOver, &TriangleInRect);
	}else if (o->GetName() == "MultiView"){
		// vertex selection from trias
		foreach(data->Vertex, v)
			v.is_selected = false;
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
				if (t.is_selected)
					for (int k=0;k<3;k++)
						data->Vertex[t.Vertex[k]].is_selected = true;
		// surface selection from trias
		foreach(data->Surface, s){
			s.is_selected = true;
			foreach(s.Triangle, t)
				s.is_selected &= t.is_selected;
		}
	}
	FillSelectionBuffers();
}



void ModeModelMeshTriangle::OnRightButtonDown()
{
}



void ModeModelMeshTriangle::OnRightButtonUp()
{
}



void ModeModelMeshTriangle::OnCommand(const string & id)
{
}



void ModeModelMeshTriangle::Draw()
{
	FillSelectionBuffers();

	if (data->GetNumMarkedVertices() > 0){
		NixDrawStr(20, 100, format(_("vert: %d"), data->GetNumMarkedVertices()));
		NixDrawStr(20, 120, format(_("tria: %d"), data->GetNumMarkedTriangles()));
		NixDrawStr(20, 140, format(_("surf: %d"), data->GetNumMarkedSurfaces()));
	}
}



void ModeModelMeshTriangle::OnMiddleButtonUp()
{
}



void ModeModelMeshTriangle::OnKeyUp()
{
}



void ModeModelMeshTriangle::OnLeftButtonUp()
{
}



void ModeModelMeshTriangle::OnLeftButtonDown()
{
}



void ModeModelMeshTriangle::OnMouseMove()
{
}



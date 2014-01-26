/*
 * ModeModelMeshTexture.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshTexture", _parent, ed->multi_view_2d, "menu_model"),
	Observable("ModelMeshTexture")
{
	CurrentTextureLevel = 0;
}



ModeModelMeshTexture::~ModeModelMeshTexture()
{
}


void ModeModelMeshTexture::FetchData()
{
	skin_vertex.clear();
	foreach(ModelSurface &surf, data->Surface)
		foreach(ModelPolygon &t, surf.Polygon){
			if (t.Material != mode_model_mesh->CurrentMaterial)
				continue;
			ModelSkinVertexDummy v;
			v.m_delta = v.m_old = false;
			v.is_special = false;
			v.view_stage = t.view_stage;
			for (int k=0;k<t.Side.num;k++){
				v.is_selected = data->Vertex[t.Side[k].Vertex].is_selected;
				v.view_stage = t.view_stage;
				v.pos = t.Side[k].SkinVertex[CurrentTextureLevel];
				skin_vertex.add(v);
			}
		}

	multi_view->ClearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->AddData(	MVDModelSkinVertex,
			skin_vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
}

int ModeModelMeshTexture::GetNumSelected()
{
	int r = 0;
	foreach(ModelSkinVertexDummy &v, skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}


void ModeModelMeshTexture::OnStart()
{
	multi_view->view_stage = ed->multi_view_3d->view_stage;
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->allow_rect = true;

	FetchData();

	Observer::Subscribe(data);
	Observer::Subscribe(multi_view, "SelectionChange");

	/*ed->SetTarget("root_table", 0);
	ed->AddControlTable("", 1, 0, 1, 5, "side_table");
	ed->DeleteControl("side_table");
	ed->EmbedDialog()*/

	mode_model_mesh->ShowMaterialDialog();
}



void ModeModelMeshTexture::OnEnd()
{
	Observer::Unsubscribe(data);
	Observer::Unsubscribe(multi_view);
	multi_view->ClearData(NULL);
	skin_vertex.clear();
}

#define cur_tex			data->Material[mode_model_mesh->CurrentMaterial].Texture[CurrentTextureLevel]


void ModeModelMeshTexture::OnDrawWin(MultiView::Window *win)
{
	rect s,r;
	color c;

	vector a = win->Unproject(v_0);
	vector b = win->Unproject(vector((float)MaxX,(float)MaxY,0));

	s.x1=a.x;
	s.x2=b.x;
	s.y1=a.y;
	s.y2=b.y;

	NixEnableLighting(false);

	if (true){//mul->FXEnabled){
		// background pattern to show transparency
		color c1 = Black;
		color c2 = color(1,0.5f,0.5f,0.5f);
		for (int i=0;i<16;i++)
			for (int j=0;j<16;j++){
				r=rect(	(float)i/16.0f*NixTargetWidth,
						(float)(i+1)/16.0f*NixTargetWidth,
						(float)j/16.0f*NixTargetHeight,
						(float)(j+1)/16.0f*NixTargetHeight);
				NixSetColor(((i+j)%2==0) ? c1 : c2);
				NixDraw2D(r_id, r, 0.999f );
			}
		NixSetAlphaSD(AlphaSourceAlpha,AlphaSourceInvAlpha);
	}
	NixSetColor(color(1,0.8f,0.8f,0.8f));
	NixSetTexture(cur_tex);
	NixDraw2D(s, NixTargetRect, 0.99f);
	NixSetTexture(NULL);
	NixSetAlphaM(AlphaNone);

	// rectangle of unity
	a = win->Project(v_0);
	b = win->Project(vector(1, 1, 0));
	NixSetColor(Red);
	NixDrawLine(a.x, a.y, b.x, a.y, 0.98f);
	NixDrawLine(b.x, a.y, b.x, b.y, 0.98f);
	NixDrawLine(a.x, a.y, a.x, b.y, 0.98f);
	NixDrawLine(a.x, b.y, b.x, b.y, 0.98f);
	NixSetColor(White);

	// draw triangles (outlines) of current material
	foreach(ModelSurface &surf, data->Surface)
		foreach(ModelPolygon &t, surf.Polygon){
			if (t.Material != mode_model_mesh->CurrentMaterial)
				continue;
			if (t.view_stage < multi_view->view_stage)
				continue;
			Array<vector> v;
			for (int k=0;k<t.Side.num;k++)
				v.add(win->Project(t.Side[k].SkinVertex[CurrentTextureLevel]));
			v.add(v[0]);
			for (int k=0;k<t.Side.num;k++)
				NixDrawLine(	v[k].x,v[k].y,
								v[k+1].x,v[k+1].y,
								0.9f);
		}
}



void ModeModelMeshTexture::OnDraw()
{
	if (data->GetNumSelectedVertices() > 0){
		ed->DrawStr(20, 160, format(_("skin: %d"), GetNumSelected()));
	}
}

void ModeModelMeshTexture::SetCurrentTextureLevel(int level)
{
	//if (CurrentTextureLevel == level)
	//	return;
	CurrentTextureLevel = level;
	FetchData();
	Notify("Change");
}

void ModeModelMeshTexture::OnUpdate(Observable *o)
{
	// consistency checks
	if (CurrentTextureLevel >= data->Material[mode_model_mesh->CurrentMaterial].NumTextures)
		SetCurrentTextureLevel(data->Material[mode_model_mesh->CurrentMaterial].NumTextures - 1);

	if (o->GetName() == "Data"){

		if (o->GetMessage() == "SkinChange"){
			int svi = 0;
			foreach(ModelSurface &surf, data->Surface)
				foreach(ModelPolygon &t, surf.Polygon){
					if (t.Material != mode_model_mesh->CurrentMaterial)
						continue;
					for (int k=0;k<t.Side.num;k++)
						skin_vertex[svi ++].pos = t.Side[k].SkinVertex[CurrentTextureLevel];
				}
		}else if (o->GetMessage() == "Change"){

			FetchData();
		}
	}else if (o->GetName() == "MultiView"){
		//data->SelectionTrianglesFromVertices();
		//data->SelectionSurfacesFromTriangles();
	}else if (o->GetName() == "ModelMesg"){
	}
	//mode_model_mesh_triangle->FillSelectionBuffers();
}


// used by actions
void ModeModelMeshTexture::GetSelectedSkinVertices(Array<int> & surf, Array<int> &tria, Array<int> & index)
{
	int i = 0;
	foreachi(ModelSurface &s, data->Surface, si)
		foreachi(ModelPolygon &t, s.Polygon, ti)
			if (t.Material == mode_model_mesh->CurrentMaterial){
				for (int k=0;k<t.Side.num;k++){
					if (skin_vertex[i].is_selected){
						index.add(k);
						surf.add(si);
						tria.add(ti);
					}
					i ++;
				}
			}
}

/*
 * ModeModelMeshTexture.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshTexture";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_2d;
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
}



ModeModelMeshTexture::~ModeModelMeshTexture()
{
}



void ModeModelMeshTexture::Start()
{
	skin_vertex.clear();
	foreach(data->Surface, surf)
		foreach(surf.Triangle, t){
			if (t.Material != data->CurrentMaterial)
				continue;
			ModeModelSkinVertexDummy v;
			v.m_delta = v.m_old = false;
			v.is_special = false;
			v.view_stage = t.view_stage;
			for (int k=0;k<3;k++){
				v.is_selected = data->Vertex[t.Vertex[k]].is_selected;
				v.pos = t.SkinVertex[data->CurrentTextureLevel][k];
				skin_vertex.add(v);
			}
		}
	OnUpdate(data);
}



void ModeModelMeshTexture::End()
{
	skin_vertex.clear();
}

#define cur_tex			data->Material[data->CurrentMaterial].Texture[data->CurrentTextureLevel]


void ModeModelMeshTexture::DrawWin(int win, irect dest)
{
	rect s,r;
	color c;

	vector a = multi_view->VecUnProject(v0, win);
	vector b = multi_view->VecUnProject(vector((float)MaxX,(float)MaxY,0),win);

	s.x1=a.x;
	s.x2=b.x;
	s.y1=a.y;
	s.y2=b.y;

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
				NixDraw2D( -1, ((i+j)%2==0) ? c1 : c2, r01, r, 0.999f );
			}
		NixSetAlphaSD(AlphaSourceAlpha,AlphaSourceInvAlpha);
	}
	NixDraw2D(cur_tex, color(1,0.7f,0.7f,0.7f), s, NixTargetRect, 0.99f);
	NixSetAlphaM(AlphaNone);

	// draw triangles (outlines) of current material
	foreach(data->Surface, surf)
		foreach(surf.Triangle, t){
			if (t.Material != data->CurrentMaterial)
				continue;
			if (t.view_stage < data->ViewStage)
				continue;
			vector a,b,c;
			a = multi_view->VecProject(t.SkinVertex[data->CurrentTextureLevel][0],win);
			b = multi_view->VecProject(t.SkinVertex[data->CurrentTextureLevel][1],win);
			c = multi_view->VecProject(t.SkinVertex[data->CurrentTextureLevel][2],win);
			NixDrawLine(	a.x,a.y,
							b.x,b.y,
							White,0.9f);
			NixDrawLine(	c.x,c.y,
							b.x,b.y,
							White,0.9f);
			NixDrawLine(	a.x,a.y,
							c.x,c.y,
							White,0.9f);
		}

	ed->DrawStr(180, MaxY - 20, format("%d von %d: %s", data->CurrentTextureLevel + 1, data->Material[data->CurrentMaterial].NumTextures,
			data->Material[data->CurrentMaterial].TextureFile[data->CurrentTextureLevel].c_str()));
}



void ModeModelMeshTexture::Draw()
{
	if (data->GetNumMarkedVertices() > 0){
		NixDrawStr(20, 100, format(_("vert: %d"), data->GetNumMarkedVertices()));
		NixDrawStr(20, 120, format(_("tria: %d"), data->GetNumMarkedTriangles()));
		NixDrawStr(20, 140, format(_("surf: %d"), data->GetNumMarkedSurfaces()));
		NixDrawStr(20, 160, format(_("skin: %d"), data->GetNumMarkedSkinVertices()));
	}
}



void ModeModelMeshTexture::OnUpdate(Observable *o)
{
	if (this != ed->cur_mode)
		return;
	if (o->GetName() == "Data"){


		int svi = 0;
		foreach(data->Surface, surf)
			foreach(surf.Triangle, t){
				if (t.Material != data->CurrentMaterial)
					continue;
				for (int k=0;k<3;k++)
					skin_vertex[svi ++].pos = t.SkinVertex[data->CurrentTextureLevel][k];
			}

		multi_view->ResetData(data);
		mode_model_mesh->ApplyRightMouseFunction(multi_view);
		multi_view->MVRectable = true;
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		//CModeAll::SetMultiViewFunctions(&StartChanging, &EndChanging, &Change);
		multi_view->SetData(	MVDModelSkinVertex,
				skin_vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
		/*// tria selection from vertices
		foreach(data->Surface, s)
			foreach(s.Triangle, t)
				t.is_selected = ((data->Vertex[t.Vertex[0]].is_selected) and (data->Vertex[t.Vertex[1]].is_selected) and (data->Vertex[t.Vertex[2]].is_selected));
		// surface selection from trias
		foreach(data->Surface, s){
			s.is_selected = true;
			foreach(s.Triangle, t)
				s.is_selected &= t.is_selected;
		}*/
	}
	//mode_model_mesh_triangle->FillSelectionBuffers();
}



void ModeModelMeshTexture::GetSelectedSkinVertices(Array<int> & surf, Array<int> & index)
{
	int i = 0;
	foreachi(data->Surface, s, si)
		foreach(s.Triangle, t)
			if (t.Material == data->CurrentMaterial){
				for (int k=0;k<3;k++){
					if (skin_vertex[i].is_selected){
						index.add(i);
						//old_data.add(skin_vertex[i].pos);
						surf.add(si);
					}
					i ++;
				}
			}
}

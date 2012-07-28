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
#include "../Dialog/ModelTextureLevelDialog.h"

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshTexture";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_2d;
}



ModeModelMeshTexture::~ModeModelMeshTexture()
{
}



void ModeModelMeshTexture::OnStart()
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

	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	OnUpdate(data);

	dialog = new ModelTextureLevelDialog(ed, true, data);
	dialog->Update();
}



void ModeModelMeshTexture::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	skin_vertex.clear();
	delete(dialog);
}

#define cur_tex			data->Material[data->CurrentMaterial].Texture[data->CurrentTextureLevel]


void ModeModelMeshTexture::OnDrawWin(int win, irect dest)
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
				NixSetColor(((i+j)%2==0) ? c1 : c2);
				NixDraw2D(r_id, r, 0.999f );
			}
		NixSetAlphaSD(AlphaSourceAlpha,AlphaSourceInvAlpha);
	}
	NixSetColor(color(1,0.7f,0.7f,0.7f));
	NixSetTexture(cur_tex);
	NixDraw2D(s, NixTargetRect, 0.99f);
	NixSetTexture(-1);
	NixSetAlphaM(AlphaNone);

	// rectangle of unity
	a = multi_view->VecProject(v0, win);
	b = multi_view->VecProject(vector(1, 1, 0), win);
	NixSetColor(Red);
	NixDrawLine(a.x, a.y, b.x, a.y, 0.98f);
	NixDrawLine(b.x, a.y, b.x, b.y, 0.98f);
	NixDrawLine(a.x, a.y, a.x, b.y, 0.98f);
	NixDrawLine(a.x, b.y, b.x, b.y, 0.98f);
	NixSetColor(White);

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
							0.9f);
			NixDrawLine(	c.x,c.y,
							b.x,b.y,
							0.9f);
			NixDrawLine(	a.x,a.y,
							c.x,c.y,
							0.9f);
		}

	ed->DrawStr(180, MaxY - 20, format("%d von %d: %s", data->CurrentTextureLevel + 1, data->Material[data->CurrentMaterial].NumTextures,
			data->Material[data->CurrentMaterial].TextureFile[data->CurrentTextureLevel].c_str()));
}



void ModeModelMeshTexture::OnDraw()
{
	if (data->GetNumMarkedVertices() > 0){
		NixDrawStr(20, 160, format(_("skin: %d"), data->GetNumMarkedSkinVertices()));
	}
}



void ModeModelMeshTexture::OnUpdate(Observable *o)
{
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
		//data->SelectionTrianglesFromVertices();
		//data->SelectionSurfacesFromTriangles();
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

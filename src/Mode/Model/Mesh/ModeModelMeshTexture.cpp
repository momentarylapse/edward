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
	current_texture_level = 0;
}


void ModeModelMeshTexture::fetchData()
{
	skin_vertex.clear();
	foreach(ModelSurface &surf, data->surface)
		foreach(ModelPolygon &t, surf.polygon){
			if (t.material != mode_model_mesh->current_material)
				continue;
			ModelSkinVertexDummy v;
			v.m_delta = v.m_old = false;
			v.is_special = false;
			v.view_stage = t.view_stage;
			for (int k=0;k<t.side.num;k++){
				v.is_selected = data->vertex[t.side[k].vertex].is_selected;
				v.view_stage = t.view_stage;
				v.pos = t.side[k].skin_vertex[current_texture_level];
				skin_vertex.add(v);
			}
		}

	multi_view->clearData(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->addData(	MVD_MODEL_SKIN_VERTEX,
			skin_vertex,
			NULL,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

int ModeModelMeshTexture::getNumSelected()
{
	int r = 0;
	foreach(ModelSkinVertexDummy &v, skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}


void ModeModelMeshTexture::onStart()
{
	multi_view->view_stage = ed->multi_view_3d->view_stage;
	mode_model_mesh->applyMouseFunction(multi_view);
	multi_view->allow_rect = true;

	fetchData();

	Observer::subscribe(data);
	Observer::subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);

	/*ed->SetTarget("root_table", 0);
	ed->AddControlTable("", 1, 0, 1, 5, "side_table");
	ed->DeleteControl("side_table");
	ed->EmbedDialog()*/

	mode_model_mesh->showMaterialDialog();
}



void ModeModelMeshTexture::onEnd()
{
	Observer::unsubscribe(data);
	Observer::unsubscribe(multi_view);
	skin_vertex.clear();
}

#define cur_tex			data->material[mode_model_mesh->current_material].texture[current_texture_level]


void ModeModelMeshTexture::onDrawWin(MultiView::Window *win)
{
	rect s,r;
	color c;

	vector a = win->unproject(v_0);
	vector b = win->unproject(vector((float)MaxX,(float)MaxY,0));

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
	a = win->project(v_0);
	b = win->project(vector(1, 1, 0));
	NixSetColor(Red);
	NixDrawLine(a.x, a.y, b.x, a.y, 0.98f);
	NixDrawLine(b.x, a.y, b.x, b.y, 0.98f);
	NixDrawLine(a.x, a.y, a.x, b.y, 0.98f);
	NixDrawLine(a.x, b.y, b.x, b.y, 0.98f);
	NixSetColor(White);

	// draw triangles (outlines) of current material
	foreach(ModelSurface &surf, data->surface)
		foreach(ModelPolygon &t, surf.polygon){
			if (t.material != mode_model_mesh->current_material)
				continue;
			if (t.view_stage < multi_view->view_stage)
				continue;
			Array<vector> v;
			for (int k=0;k<t.side.num;k++)
				v.add(win->project(t.side[k].skin_vertex[current_texture_level]));
			v.add(v[0]);
			for (int k=0;k<t.side.num;k++)
				NixDrawLine(	v[k].x,v[k].y,
								v[k+1].x,v[k+1].y,
								0.9f);
		}
}



void ModeModelMeshTexture::onDraw()
{
	if (data->GetNumSelectedVertices() > 0){
		ed->drawStr(20, 160, format(_("skin: %d"), getNumSelected()));
	}
}

void ModeModelMeshTexture::setCurrentTextureLevel(int level)
{
	//if (CurrentTextureLevel == level)
	//	return;
	current_texture_level = level;
	fetchData();
	notify();
}

void ModeModelMeshTexture::onUpdate(Observable *o, const string &message)
{
	// consistency checks
	if (current_texture_level >= data->material[mode_model_mesh->current_material].num_textures)
		setCurrentTextureLevel(data->material[mode_model_mesh->current_material].num_textures - 1);

	if (o == data){

		if (message == DataModel::MESSAGE_SKIN_CHANGE){
			int svi = 0;
			foreach(ModelSurface &surf, data->surface)
				foreach(ModelPolygon &t, surf.polygon){
					if (t.material != mode_model_mesh->current_material)
						continue;
					for (int k=0;k<t.side.num;k++)
						skin_vertex[svi ++].pos = t.side[k].skin_vertex[current_texture_level];
				}
		}else if (message == data->MESSAGE_CHANGE){

			fetchData();
		}
	}else if (o == multi_view){
		//data->SelectionTrianglesFromVertices();
		//data->SelectionSurfacesFromTriangles();
	}
	//mode_model_mesh_triangle->FillSelectionBuffers();
}


// used by actions
void ModeModelMeshTexture::getSelectedSkinVertices(Array<int> & surf, Array<int> &tria, Array<int> & index)
{
	int i = 0;
	foreachi(ModelSurface &s, data->surface, si)
		foreachi(ModelPolygon &t, s.polygon, ti)
			if (t.material == mode_model_mesh->current_material){
				for (int k=0;k<t.side.num;k++){
					if (skin_vertex[i].is_selected){
						index.add(k);
						surf.add(si);
						tria.add(ti);
					}
					i ++;
				}
			}
}

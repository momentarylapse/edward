/*
 * ModeModelMeshTexture.cpp
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../lib/nix/nix.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"
#include "../Dialog/ModelMaterialDialog.h"

const string ModeModelMeshTexture::MESSAGE_TEXTURE_LEVEL_CHANGE = "TextureLevelChange";

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshTexture", _parent, ed->multi_view_2d, "menu_model"),
	Observable("ModelMeshTexture")
{
	current_texture_level = 0;
	dialog = nullptr;
}


void ModeModelMeshTexture::fetchData()
{
	skin_vertex.clear();
	for (ModelSurface &surf: data->surface)
		for (ModelPolygon &t: surf.polygon){
			if (t.material != mode_model_mesh->current_material)
				continue;
			ModelSkinVertexDummy v;
			v.m_delta = v.m_old = false;
			v.is_special = false;
			v.view_stage = t.view_stage;
			for (int k=0;k<t.side.num;k++){
				v.is_selected = t.is_selected; //data->vertex[t.side[k].vertex].is_selected;
				v.view_stage = t.view_stage;
				v.pos = t.side[k].skin_vertex[current_texture_level];
				skin_vertex.add(v);
			}
		}

	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(	MVD_MODEL_SKIN_VERTEX,
			skin_vertex,
			NULL,
			MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
}

int ModeModelMeshTexture::getNumSelected()
{
	int r = 0;
	for (ModelSkinVertexDummy &v: skin_vertex)
		if (v.is_selected)
			r ++;
	return r;
}


void ModeModelMeshTexture::on_start()
{
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-texture-toolbar");

	multi_view->view_stage = ed->multi_view_3d->view_stage;
	mode_model_mesh->apply_mouse_function(multi_view);

	fetchData();

	Observer::subscribe(data);
	Observer::subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);

	/*ed->SetTarget("root_table", 0);
	ed->AddControlTable("", 1, 0, 1, 5, "side_table");
	ed->DeleteControl("side_table");
	ed->EmbedDialog()*/

	dialog = new ModelMaterialDialog(data, false);
	ed->embed(dialog, "root-table", 1, 0);
}



void ModeModelMeshTexture::on_end()
{
	Observer::unsubscribe(data);
	Observer::unsubscribe(multi_view);
	skin_vertex.clear();
	delete dialog;
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar"); // -> mesh
}

#define cur_tex			data->material[mode_model_mesh->current_material]->texture_levels[current_texture_level]->texture


void ModeModelMeshTexture::on_draw_win(MultiView::Window *win)
{
	rect s,r;
	color c;

	vector a = win->unproject(v_0);
	vector b = win->unproject(vector((float)nix::target_width,(float)nix::target_height,0));

	nix::SetZ(false, false);
	s.x1=a.x;
	s.x2=b.x;
	s.y1=a.y;
	s.y2=b.y;

	nix::EnableLighting(false);

	if (true){//mul->FXEnabled){
		// background pattern to show transparency
		color c1 = Black;
		color c2 = color(1,0.5f,0.5f,0.5f);
		for (int i=0;i<16;i++)
			for (int j=0;j<16;j++){
				r=rect(	(float)i/16.0f*nix::target_width,
						(float)(i+1)/16.0f*nix::target_width,
						(float)j/16.0f*nix::target_height,
						(float)(j+1)/16.0f*nix::target_height);
				nix::SetColor(((i+j)%2==0) ? c1 : c2);
				nix::Draw2D(rect::ID, r, 0.999f );
			}
		nix::SetAlphaSD(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
	}
	nix::SetColor(color(1,0.8f,0.8f,0.8f));
	nix::SetTexture(cur_tex);
	nix::Draw2D(s, nix::target_rect, 0.99f);
	nix::SetTexture(NULL);
	nix::SetAlphaM(ALPHA_NONE);

	// rectangle of unity
	a = win->project(v_0);
	b = win->project(vector(1, 1, 0));
	nix::SetColor(Red);
	nix::DrawLine(a.x, a.y, b.x, a.y, 0.98f);
	nix::DrawLine(b.x, a.y, b.x, b.y, 0.98f);
	nix::DrawLine(a.x, a.y, a.x, b.y, 0.98f);
	nix::DrawLine(a.x, b.y, b.x, b.y, 0.98f);
	nix::SetColor(White);

	// draw triangles (outlines) of current material
	for (ModelSurface &surf: data->surface)
		for (ModelPolygon &t: surf.polygon){
			if (t.material != mode_model_mesh->current_material)
				continue;
			if (t.view_stage < multi_view->view_stage)
				continue;
			Array<vector> v;
			for (int k=0;k<t.side.num;k++)
				v.add(win->project(t.side[k].skin_vertex[current_texture_level]));
			v.add(v[0]);
			for (int k=0;k<t.side.num;k++)
				nix::DrawLine(	v[k].x,v[k].y,
								v[k+1].x,v[k+1].y,
								0.9f);
		}
}



void ModeModelMeshTexture::on_draw()
{
	if (data->getNumSelectedVertices() > 0){
		draw_str(20, 160, format(_("skin: %d"), getNumSelected()));
	}
	if (data->getNumSelectedVertices() > 0){
		int nv = data->getNumSelectedVertices();
		int ne = data->getNumSelectedEdges();
		int np = data->getNumSelectedPolygons();
		int ns = data->getNumSelectedSurfaces();
		draw_str(10, nix::target_height - 25, format("selected: %d vertices, %d edges, %d polygons, %d surfaces", nv, ne, np, ns));
	}
}

void ModeModelMeshTexture::on_selection_change()
{
	//selection_mode->updateSelection();
	//fillSelectionBuffer(data->vertex);

	int nn = 0;
	for (ModelSurface &surf: data->surface)
		for (ModelPolygon &t: surf.polygon){
			if (t.material != mode_model_mesh->current_material)
				continue;
			t.is_selected = true;
			for (int k=0;k<t.side.num;k++){
				t.is_selected &= skin_vertex[nn].is_selected;
				/*if (skin_vertex[nn].is_selected)
					data->vertex[t.side[k].vertex].is_selected = true;*/
				nn ++;
			}
		}

	data->selectionFromPolygons();

}

void ModeModelMeshTexture::setCurrentTextureLevel(int level)
{
	//if (CurrentTextureLevel == level)
	//	return;
	current_texture_level = level;
	fetchData();
	notify(MESSAGE_TEXTURE_LEVEL_CHANGE);
	multi_view->force_redraw();
}

void ModeModelMeshTexture::on_update(Observable *o, const string &message)
{
	// consistency checks
	if (current_texture_level >= data->material[mode_model_mesh->current_material]->texture_levels.num)
		setCurrentTextureLevel(data->material[mode_model_mesh->current_material]->texture_levels.num - 1);

	if (o == data){

		if (message == DataModel::MESSAGE_SKIN_CHANGE){
			int svi = 0;
			for (ModelSurface &surf: data->surface)
				for (ModelPolygon &t: surf.polygon){
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

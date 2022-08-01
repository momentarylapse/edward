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
#include "../../../MultiView/ColorScheme.h"
#include "../../../lib/nix/nix.h"
#include "../../../Data/Model/ModelSelection.h"
#include "../../../Data/Model/ModelMesh.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"
#include "../Dialog/ModelMaterialDialog.h"

const string ModeModelMeshTexture::State::MESSAGE_TEXTURE_LEVEL_CHANGE = "TextureLevelChange";

ModeModelMeshTexture *mode_model_mesh_texture = NULL;

ModeModelMeshTexture::ModeModelMeshTexture(ModeBase *_parent, MultiView::MultiView *mv) :
	Mode<DataModel>("ModelMeshTexture", _parent, mv, "menu_model")
{
	current_texture_level = 0;
	dialog = nullptr;
}


void ModeModelMeshTexture::fetchData() {
	skin_vertex.clear();
	for (ModelPolygon &t: data->mesh->polygon) {
		if (t.material != mode_model_mesh->current_material)
			continue;
		ModelSkinVertexDummy v;
		v.m_delta = v.m_old = false;
		v.is_special = false;
		v.view_stage = t.view_stage;
		for (int k=0; k<t.side.num; k++) {
			v.is_selected = t.is_selected; //data->vertex[t.side[k].vertex].is_selected;
			v.view_stage = t.view_stage;
			v.pos = t.side[k].skin_vertex[current_texture_level];
			skin_vertex.add(v);
		}
	}

	multi_view->clear_data(data);
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(MVD_MODEL_SKIN_VERTEX,
			skin_vertex,
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


void ModeModelMeshTexture::on_start() {
	ed->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-texture-toolbar");

	multi_view->view_stage = parent->multi_view->view_stage;
	mode_model_mesh->apply_mouse_function(multi_view);

	fetchData();

	data->subscribe(this, [=]{ on_data_skin_change(); }, data->MESSAGE_SKIN_CHANGE);
	data->subscribe(this, [=]{ on_data_change(); }, data->MESSAGE_CHANGE);
	multi_view->subscribe(this, [=]{
		//data->SelectionTrianglesFromVertices();
		//data->SelectionSurfacesFromTriangles();
		//mode_model_mesh_triangle->FillSelectionBuffers();
	}, multi_view->MESSAGE_SELECTION_CHANGE);

	/*ed->SetTarget("root_table", 0);
	ed->AddControlTable("", 1, 0, 1, 5, "side_table");
	ed->DeleteControl("side_table");
	ed->EmbedDialog()*/

	dialog = new ModelMaterialDialog(data, false);
	ed->set_side_panel(dialog);
}



void ModeModelMeshTexture::on_end() {
	data->unsubscribe(this);
	multi_view->unsubscribe(this);
	skin_vertex.clear();
	ed->set_side_panel(nullptr);
	ed->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-mesh-toolbar"); // -> mesh
}

#define cur_tex			data->material[mode_model_mesh->current_material]->texture_levels[current_texture_level]->texture.get()


void ModeModelMeshTexture::on_draw_win(MultiView::Window *win)
{
	rect s,r;
	color c;

	vec3 a = win->unproject(v_0);
	vec3 b = win->unproject(vec3((float)nix::target_width,(float)nix::target_height,0));

	nix::set_z(false, false);
	s.x1=a.x;
	s.x2=b.x;
	s.y1=a.y;
	s.y2=b.y;

	nix::set_shader(nix::Shader::default_2d);

	if (true){//mul->FXEnabled){
		// background pattern to show transparency
		color c1 = Black;
		color c2 = color(1,0.5f,0.5f,0.5f);
		for (int i=0;i<16;i++)
			for (int j=0;j<16;j++){
				r=rect(	(float)i/16.0f,
						(float)(i+1)/16.0f,
						(float)j/16.0f,
						(float)(j+1)/16.0f);
				set_color(((i+j)%2==0) ? c1 : c2);
				draw_2d(rect::ID, r, 0.999f );
			}
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	}
	set_color(color(1,0.8f,0.8f,0.8f));
	nix::set_texture(cur_tex);
	draw_2d(s, s, 0.99f);
	nix::set_texture(nullptr);
	nix::disable_alpha();

	// rectangle of unity
	a = v_0;
	b = vec3(1, 1, 0);
	set_color(Red);
	set_line_width(scheme.LINE_WIDTH_THIN);
	draw_line_2d(a.x, a.y, b.x, a.y, 0.98f);
	draw_line_2d(b.x, a.y, b.x, b.y, 0.98f);
	draw_line_2d(a.x, a.y, a.x, b.y, 0.98f);
	draw_line_2d(a.x, b.y, b.x, b.y, 0.98f);


	// draw triangles (outlines) of current material
	set_color(White);
	for (ModelPolygon &t: data->mesh->polygon) {
		if (t.material != mode_model_mesh->current_material)
			continue;
		if (t.view_stage < multi_view->view_stage)
			continue;
		Array<vec3> v;
		for (int k=0;k<t.side.num;k++)
			v.add(t.side[k].skin_vertex[current_texture_level]);
		v.add(v[0]);
		for (int k=0;k<t.side.num;k++)
			draw_line_2d(	v[k].x,v[k].y,
							v[k+1].x,v[k+1].y,
							0.9f);
	}
}



void ModeModelMeshTexture::on_draw() {
	nix::set_shader(nix::Shader::default_2d);
	auto s = data->get_selection();
	/*if (data->getNumSelectedVertices() > 0){
		draw_str(20, 160, format(_("skin: %d"), getNumSelected()));
	}*/
	if (s.vertex.num > 0){
		draw_str(10, nix::target_height - 25, format("selected: %d vertices, %d edges, %d polygons", s.vertex.num, s.edge.num, s.polygon.num));
	}
}

void ModeModelMeshTexture::on_selection_change()
{
	//selection_mode->updateSelection();
	//fillSelectionBuffer(data->vertex);

	int nn = 0;
	for (ModelPolygon &t: data->mesh->polygon) {
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

void ModeModelMeshTexture::set_current_texture_level(int level) {
	//if (CurrentTextureLevel == level)
	//	return;
	current_texture_level = level;
	fetchData();
	state.notify(state.MESSAGE_TEXTURE_LEVEL_CHANGE);
	multi_view->force_redraw();
}

void ModeModelMeshTexture::on_data_skin_change() {
	int svi = 0;
	for (ModelPolygon &t: data->mesh->polygon) {
		if (t.material != mode_model_mesh->current_material)
			continue;
		for (int k=0;k<t.side.num;k++)
			skin_vertex[svi ++].pos = t.side[k].skin_vertex[current_texture_level];
	}
}

void ModeModelMeshTexture::on_data_change() {
	// consistency checks
	if (current_texture_level >= data->material[mode_model_mesh->current_material]->texture_levels.num)
		set_current_texture_level(data->material[mode_model_mesh->current_material]->texture_levels.num - 1);
}


// used by actions
void ModeModelMeshTexture::getSelectedSkinVertices(Array<int> &tria, Array<int> & index)
{
	int i = 0;
	foreachi(ModelPolygon &t, data->mesh->polygon, ti)
		if (t.material == mode_model_mesh->current_material){
			for (int k=0;k<t.side.num;k++){
				if (skin_vertex[i].is_selected){
					index.add(k);
					tria.add(ti);
				}
				i ++;
			}
		}
}

//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeMesh.h"
#include "ModeAddVertex.h"
#include "ModeAddPolygon.h"
#include "ModeAddCube.h"
#include "ModeAddSphere.h"
#include "ModeAddPlatonic.h"
#include "ModeAddFromLathe.h"
#include "ModePaste.h"
#include "ModeMeshSculpt.h"
#include "ModeMeshMaterial.h"
#include "../ModeModel.h"
#include "../action/mesh/ActionModelMoveSelection.h"
#include "../data/ModelMesh.h"
#include "../dialog/ModelPropertiesDialog.h"
#include <Session.h>
#include <data/mesh/GeometryCylinder.h>
#include <data/mesh/GeometrySphere.h>
#include <helper/ResourceManager.h>
#include <lib/base/iter.h>
#include <lib/image/Painter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/controls/Toolbar.h>
#include <view/ActionController.h>
#include <view/MultiView.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <data/mesh/VertexStagingBuffer.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/MenuBar.h>
#include <mode_model/dialog/ModelMaterialSelectionDialog.h>
#include <storage/Storage.h>

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeMesh::ModeMesh(ModeModel* parent) : SubMode(parent) {
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
	vertex_buffer_physical = new VertexBuffer("3f,3f,2f");
	vertex_buffer_selection = new VertexBuffer("3f,3f,2f");
	vertex_buffer_hover = new VertexBuffer("3f,3f,2f");
	material_physical = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, color(1,1,1,0.4f), true);
	material_selection = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, Red, true);
	material_hover = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, White, true);

	temp_mesh = new ModelMesh(data);
	current_material = 0;
	current_texture_level = 0;

	presentation_mode = PresentationMode::Polygons;
}

ModeMesh::~ModeMesh() = default;

void ModeMesh::set_current_material(int index) {
	if (index == current_material or index < 0)
		return;
	set_current_texture_level(0);
	current_material = index;
	out_current_material_changed();
}

void ModeMesh::set_current_texture_level(int index) {
	if (index == current_texture_level or index < 0)
		return;
	current_texture_level = index;
	out_texture_level_changed();
}


void ModeMesh::set_edit_mesh(ModelMesh* mesh) {
	data->edit_mesh = mesh;
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &mesh->vertices},
		{MultiViewType::MODEL_POLYGON, &mesh->polygons},
	};
	update_menu();
	update_vb();
}


void ModeMesh::on_enter() {
	auto update = [this] {
		data->mesh->update_normals();
		update_vb();
		update_selection_vb();
		session->win->request_redraw();
	};

	auto win = session->win;
	auto tb = win->toolbar;
	tb->set_by_id("model-toolbar");

	auto menu_bar = (xhui::MenuBar*)win->get_control("menu");
	auto menu = xhui::create_resource_menu("menu_model");
	menu_bar->set_menu(menu);

	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	session->win->set_visible("overlay-button-grid-left", true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		select_in_rect(win, r);
	};
	multi_view->f_get_selection_box = [this] {
		return get_selection_box();
	};
	multi_view->f_create_action = [this] {
		return new ActionModelMoveSelection(data->edit_mesh, data->get_selection());
	};
	set_edit_mesh(data->mesh.get());
	multi_view->out_selection_changed >> create_sink([this] {
		on_update_selection();
	});

	win->set_target("overlay-button-grid-left");
	win->add_control("Button", "V", 0, 1, "add-vertex");
	win->set_options("add-vertex", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "P", 0, 2, "add-polygon");
	win->set_options("add-polygon", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "Q", 0, 3, "add-cube");
	win->set_options("add-cube", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "S", 0, 4, "add-sphere");
	win->set_options("add-sphere", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "P", 0, 5, "add-platonic");
	win->set_options("add-platonic", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "L", 0, 6, "add-from-lathe");
	win->set_options("add-from-lathe", "height=50,width=50,noexpandx,ignorefocus");


	event_ids.add(session->win->event("save", [this] {
		if (data->filename.is_empty())
			session->storage->save_as(data);
		else
			session->storage->save(data->filename, data);
	}));
	event_ids.add(session->win->event("save-as", [this] {
		session->storage->save_as(data);
	}));

	event_ids.add(session->win->event("mesh-visible0", [this] {
		set_edit_mesh(data->mesh.get());
	}));
	event_ids.add(session->win->event("mesh-physical", [this] {
		set_edit_mesh(data->phys_mesh.get());
	}));

	event_ids.add(session->win->event("mode_model_deform", [this] {
		session->set_mode(new ModeMeshSculpt(this));
	}));
	event_ids.add(session->win->event("mode_model_materials", [this] {
		session->set_mode(new ModeMeshMaterial(this));
	}));
	event_ids.add(session->win->event("mode_properties", [this] {
		session->win->open_dialog(new ModelPropertiesDialog(session->win, data));
	}));

	event_ids.add(session->win->event("mode_model_vertex", [this] {
		set_presentation_mode(PresentationMode::Vertices);
	}));
	event_ids.add(session->win->event("mode_model_edge", [this] {
		set_presentation_mode(PresentationMode::Edges);
	}));
	event_ids.add(session->win->event("mode_model_polygon", [this] {
		set_presentation_mode(PresentationMode::Polygons);
	}));
	event_ids.add(session->win->event("mode_model_surface", [this] {
		set_presentation_mode(PresentationMode::Surfaces);
	}));

	event_ids.add(session->win->event("add-vertex", [this] {
		session->set_mode(new ModeAddVertex(this));
	}));
	event_ids.add(session->win->event("add-polygon", [this] {
		session->set_mode(new ModeAddPolygon(this));
	}));
	event_ids.add(session->win->event("add-cube", [this] {
		session->set_mode(new ModeAddCube(this));
	}));
	event_ids.add(session->win->event("add-sphere", [this] {
		session->set_mode(new ModeAddSphere(this));
	}));
	event_ids.add(session->win->event("add-platonic", [this] {
		session->set_mode(new ModeAddPlatonic(this));
	}));
	event_ids.add(session->win->event("add-from-lathe", [this] {
		session->set_mode(new ModeAddFromLathe(this));
	}));
	event_ids.add(session->win->event("normal_this_hard", [this] {
		auto sel = data->get_selection();
		for (auto&& [i, p]: enumerate(data->mesh->polygons))
			if (sel[MultiViewType::MODEL_POLYGON].contains(i)) {
				p.smooth_group = -1;
				p.normal_dirty = true;
			}
		data->mesh->update_normals();
		data->out_changed();
	}));
	event_ids.add(session->win->event("normal_this_smooth", [this] {
		auto sel = data->get_selection();
		for (auto&& [i, p]: enumerate(data->mesh->polygons))
			if (sel[MultiViewType::MODEL_POLYGON].contains(i)) {
				p.smooth_group = 42;
				p.normal_dirty = true;
			}
		data->mesh->update_normals();
		data->out_changed();
	}));
	event_ids.add(session->win->event("choose_material", [this] {
		ModelMaterialSelectionDialog::ask(this).then([this] (int material) {
			data->apply_material(data->get_selection(), material);
		});
	}));


	data->out_changed >> create_sink(update);
	update();
	update_menu();
}

void ModeMesh::on_leave() {
	data->out_changed.unsubscribe(this);
	for (int uid: event_ids)
		session->win->remove_event_handler(uid);
	event_ids.clear();
}

void ModeMesh::set_presentation_mode(PresentationMode m) {
	presentation_mode = m;
	update_menu();
	session->win->request_redraw();
}

void ModeMesh::update_menu() {
	auto win = session->win;
	win->check("mesh-visible0", data->edit_mesh == data->mesh.get());
	win->check("mesh-physical", data->edit_mesh == data->phys_mesh.get());
	win->check("mode_model_vertex", presentation_mode == PresentationMode::Vertices);
	win->check("mode_model_edge", presentation_mode == PresentationMode::Edges);
	win->check("mode_model_polygon", presentation_mode == PresentationMode::Polygons);
	win->check("mode_model_surface", presentation_mode == PresentationMode::Surfaces);
}



void ModeMesh::on_prepare_scene(const RenderParams& params) {
}

void ModeMesh::on_draw_background(const RenderParams& params, RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low});
}

void ModeMesh::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;

	if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces or data->edit_mesh == data->phys_mesh.get()) {
		for (int i=0; i<vertex_buffers.num; i++)
			dh->draw_mesh(params, rvd, mat4::ID, vertex_buffers[i], materials[i], 0);
	}

	if (data->edit_mesh == data->phys_mesh.get())
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_physical, material_physical, 0);

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_selection, material_selection, 0);

	VertexStagingBuffer vsb;
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_POLYGON)
		data->edit_mesh->polygons[multi_view->hover->index].add_to_vertex_buffer(data->edit_mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer_hover, 1);
	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_hover, material_hover, 0);

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges or presentation_mode == PresentationMode::Polygons) {
		// backside
		// TODO draw_lines_with_color()
		dh->set_color(color(1, 0.35f, 0.35f, 0.35f));
		dh->set_line_width(1.5f);//scheme.LINE_WIDTH_THIN);

		Array<vec3> points;
		for (const auto& p: data->edit_mesh->polygons) {
			if (vec3::dot(p.temp_normal, win->direction()) >= 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->edit_mesh->vertices[p.side[k].vertex];
					const auto& b = data->edit_mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
					if (a.is_selected and b.is_selected)
						continue;
					points.add(a.pos);
					points.add(b.pos);
				}
		}
		dh->draw_lines(points, false);


		dh->set_color(color(1, 0.5f, 0.5f, 0.5f));
		dh->set_line_width(1.5f);//scheme.LINE_WIDTH_THIN);

		points.clear();
		for (const auto& p: data->edit_mesh->polygons) {
			if (vec3::dot(p.temp_normal, win->direction()) < 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->edit_mesh->vertices[p.side[k].vertex];
					const auto& b = data->edit_mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
					if (a.is_selected and b.is_selected)
						continue;
					points.add(a.pos);
					points.add(b.pos);
				}
		}
		dh->draw_lines(points, false);


		dh->set_color(Red);
		dh->set_line_width(2);//scheme.LINE_WIDTH_THIN);

		points.clear();
		for (const auto& p: data->edit_mesh->polygons) {
			//if (vec3::dot(p.temp_normal, win->dir()) < 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->edit_mesh->vertices[p.side[k].vertex];
					const auto& b = data->edit_mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
					if (!a.is_selected or !b.is_selected)
						continue;
					points.add(a.pos);
					points.add(b.pos);
				}
		}
		dh->draw_lines(points, false);
	}


	multi_view->action_controller->draw(params, rvd);
}

base::optional<string> model_selection_description(DataModel* m) {
	int nvert = 0, npoly = 0, nsphere = 0, ncyl = 0;
	auto sel = m->get_selection();
	if (sel.contains(MultiViewType::MODEL_VERTEX))
		nvert = sel[MultiViewType::MODEL_VERTEX].num;
	if (sel.contains(MultiViewType::MODEL_POLYGON))
		npoly = sel[MultiViewType::MODEL_POLYGON].num;
	if (sel.contains(MultiViewType::MODEL_BALL))
		nsphere = sel[MultiViewType::MODEL_BALL].num;
	if (sel.contains(MultiViewType::MODEL_CYLINDER))
		ncyl = sel[MultiViewType::MODEL_CYLINDER].num;
	if (nvert + npoly == 0)
		return base::None;
	Array<string> s;
	if (nvert > 0)
		s.add(format("%d vertices", nvert));
	if (npoly > 0)
		s.add(format("%d polygons", npoly));
	if (nsphere > 0)
		s.add(format("%d spheres", nsphere));
	if (ncyl > 0)
		s.add(format("%d cylinders", ncyl));
	return implode(s, ", ");
}

void ModeMesh::on_draw_post(Painter* p) {
	if (presentation_mode == PresentationMode::Vertices)
		session->drawing_helper->draw_data_points(p, multi_view->active_window, data->edit_mesh->vertices, MultiViewType::MODEL_VERTEX, multi_view->hover);

	if (auto s = model_selection_description(data))
		draw_info(p, "selected: " + *s);
}

void ModeMesh::on_update_selection() {
	update_selection_vb();
}

void ModeMesh::update_vb() {
	vertex_buffers.resize(data->material.num);
	for (int i=0; i<vertex_buffers.num; i++) {
		if (!vertex_buffers[i])
			vertex_buffers[i] = new VertexBuffer("3f,3f,2f");

		VertexStagingBuffer vsb;
		for (auto& p: data->mesh->polygons)
			if (p.material == i)
				p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
		vsb.build(vertex_buffers[i], 1);
	}

	PolygonMesh m;
	for (const auto& b: data->edit_mesh->ball)
		m.add(GeometrySphere(data->edit_mesh->vertices[b.index].pos, b.radius, 8));
	for (const auto& c: data->edit_mesh->cylinder)
		m.add(GeometryCylinder(data->edit_mesh->vertices[c.index[0]].pos, data->edit_mesh->vertices[c.index[1]].pos, c.radius, 1, 32));
	m.build(vertex_buffer_physical);

	// update material
	materials.resize(data->material.num);
	for (int i=0; i<materials.num; i++) {
		if (!materials[i])
			materials[i] = create_material(session->resource_manager, White, 0.7f, 0.2f, Black);

		materials[i]->albedo = data->material[i]->col.albedo;
		materials[i]->metal = data->material[i]->col.metal;
		materials[i]->roughness = data->material[i]->col.roughness;
		materials[i]->emission = data->material[i]->col.emission;
		materials[i]->textures.resize(data->material[i]->texture_levels.num);
		for (int k=0; k<data->material[i]->texture_levels.num; k++)
			materials[i]->textures[k] = data->material[i]->texture_levels[k]->texture;
	}
}


void ModeMesh::update_selection_vb() {
	VertexStagingBuffer vsb;
	for (auto& p: data->edit_mesh->polygons)
		if (p.is_selected)
			p.add_to_vertex_buffer(data->edit_mesh->vertices, vsb, 1);
	// TODO spheres/cylinders
	vsb.build(vertex_buffer_selection, 1);
}


base::optional<Hover> ModeMesh::get_hover(MultiViewWindow* win, const vec2& m) const {
	if (presentation_mode == PresentationMode::Vertices) {
		base::optional<Hover> h;

		//float zmin = multi_view->view_port.radius * 2;
		for (const auto& [i, v]: enumerate(data->edit_mesh->vertices)) {
			const auto pp = win->project(v.pos);
			if (pp.z <= 0 or pp.z >= 1)
				continue;
			if ((pp.xy() - m).length_fuzzy() > 10)
				continue;
			h = {MultiViewType::MODEL_VERTEX, i, v.pos};
		}
		return h;
	}
	if (presentation_mode == PresentationMode::Polygons) {
		vec3 tp;
		int index;
		if (data->edit_mesh->is_mouse_over(win, mat4::ID, m, tp, index, false))
			return Hover{MultiViewType::MODEL_POLYGON, index, tp};
	}
	return base::None;
}

void ModeMesh::select_in_rect(MultiViewWindow* win, const rect& r) {
	if (presentation_mode == PresentationMode::Vertices) {
		MultiView::select_points_in_rect(win, r, data->edit_mesh->vertices);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		MultiView::select_points_in_rect(win, r, data->edit_mesh->vertices);
		// vertices -> polygons
		for (auto& p: data->edit_mesh->polygons) {
			p.is_selected = true;
			for (const auto& s: p.side)
				p.is_selected &= data->edit_mesh->vertices[s.vertex].is_selected;
		}
	}
}

// also "self-consistency"...
base::optional<Box> ModeMesh::get_selection_box() const {
	if (presentation_mode == PresentationMode::Vertices) {
		// vertices -> polygons
		for (auto& p: data->edit_mesh->polygons) {
			p.is_selected = true;
			for (const auto& s: p.side)
				p.is_selected &= data->edit_mesh->vertices[s.vertex].is_selected;
		}
		for (auto& b: data->edit_mesh->ball)
			b.is_selected = data->edit_mesh->vertices[b.index].is_selected;
		for (auto& c: data->edit_mesh->cylinder)
			c.is_selected = data->edit_mesh->vertices[c.index[0]].is_selected and data->edit_mesh->vertices[c.index[1]].is_selected;

		return MultiView::points_get_selection_box(data->edit_mesh->vertices);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		// polygons -> vertices
		for (auto& v: data->edit_mesh->vertices)
			v.is_selected = false;
		for (auto& p: data->edit_mesh->polygons)
			for (const auto& s: p.side)
				data->edit_mesh->vertices[s.vertex].is_selected |= p.is_selected;

		return MultiView::points_get_selection_box(data->edit_mesh->vertices);
	}
	return base::None;
}



void ModeMesh::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_MODEL);
	if (id == "open")
		session->universal_open(FD_MODEL);
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
}

void ModeMesh::copy() {
	temp_mesh->clear();
	temp_mesh->add(data->edit_mesh->copy_geometry());
	if (temp_mesh->is_empty())
		session->set_message("nothing selected");
	else
		session->set_message("copied: " + *model_selection_description(data));
}

void ModeMesh::paste() {
	multi_view->clear_selection();
	if (temp_mesh->is_empty()) {
		session->set_message("nothing to paste");
	} else {
		session->set_mode(new ModePaste(this));
	}
}


void ModeMesh::on_key_down(int key) {
	if (key == xhui::KEY_DELETE or key == xhui::KEY_BACKSPACE) {
		if (auto s = model_selection_description(data)) {
			data->delete_selection(data->get_selection(), presentation_mode == PresentationMode::Vertices);
			session->set_message("deleted: " + *s);
		} else {
			session->set_message("nothing selected");
		}
	}
}

void ModeMesh::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeMesh::optimize_view() {
	multi_view->view_port.suggest_for_box(data->bounding_box());
}




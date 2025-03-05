//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeMesh.h"
#include "ModeAddVertex.h"
#include "ModeAddPolygon.h"
#include "ModeAddCube.h"
#include "ModeAddSphere.h"
#include "ModeAddPlatonic.h"
#include "ModePaste.h"
#include "ModeMeshSculpt.h"
#include "../ModeModel.h"
#include "../action/mesh/ActionModelMoveSelection.h"
#include "../data/ModelMesh.h"
#include "../dialog/ModelPropertiesDialog.h"
#include <Session.h>
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

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeMesh::ModeMesh(ModeModel* parent) : Mode(parent->session) {
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	vertex_buffer_selection = new VertexBuffer("3f,3f,2f");
	vertex_buffer_hover = new VertexBuffer("3f,3f,2f");
	material = create_material(session->resource_manager, White, 0.7f, 0.2f, Black);
	material_selection = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, Red, true);
	material_hover = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, White, true);

	temp_mesh = new ModelMesh(data);

	presentation_mode = PresentationMode::Polygons;
}

ModeMesh::~ModeMesh() = default;

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

	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
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
		return new ActionModelMoveSelection(data, data->get_selection());
	};
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &data->mesh->vertices},
		{MultiViewType::MODEL_POLYGON, &data->mesh->polygons},
	};
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

	event_ids.add(session->win->event("mode_model_deform", [this] {
		session->set_mode(new ModeMeshSculpt(this));
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
	win->check("mode_model_vertex", presentation_mode == PresentationMode::Vertices);
	win->check("mode_model_edge", presentation_mode == PresentationMode::Edges);
	win->check("mode_model_polygon", presentation_mode == PresentationMode::Polygons);
	win->check("mode_model_surface", presentation_mode == PresentationMode::Surfaces);
}



void ModeMesh::on_prepare_scene(const RenderParams& params) {
}


void ModeMesh::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
	auto dh = win->multi_view->session->drawing_helper;
	dh->clear(params, xhui::Theme::_default.background_low);


	if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer, material, 0);

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_selection, material_selection, 0);

	VertexStagingBuffer vsb;
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_POLYGON)
		data->mesh->polygons[multi_view->hover->index].add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer_hover, 1);
	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_hover, material_hover, 0);

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges or presentation_mode == PresentationMode::Polygons) {
		// backside
		// TODO draw_lines_with_color()
		dh->set_color(color(1, 0.35f, 0.35f, 0.35f));
		dh->set_line_width(1.5f);//scheme.LINE_WIDTH_THIN);

		Array<vec3> points;
		for (const auto& p: data->mesh->polygons) {
			if (vec3::dot(p.temp_normal, win->direction()) >= 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->mesh->vertices[p.side[k].vertex];
					const auto& b = data->mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
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
		for (const auto& p: data->mesh->polygons) {
			if (vec3::dot(p.temp_normal, win->direction()) < 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->mesh->vertices[p.side[k].vertex];
					const auto& b = data->mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
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
		for (const auto& p: data->mesh->polygons) {
			//if (vec3::dot(p.temp_normal, win->dir()) < 0)
				for (int k=0; k<p.side.num; k++) {
					const auto& a = data->mesh->vertices[p.side[k].vertex];
					const auto& b = data->mesh->vertices[p.side[(k + 1) % p.side.num].vertex];
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
	int nvert = 0, npoly = 0;
	auto sel = m->get_selection();
	if (sel.contains(MultiViewType::MODEL_VERTEX))
		nvert = sel[MultiViewType::MODEL_VERTEX].num;
	if (sel.contains(MultiViewType::MODEL_POLYGON))
		npoly = sel[MultiViewType::MODEL_POLYGON].num;
	if (nvert + npoly == 0)
		return base::None;
	Array<string> s;
	if (nvert > 0)
		s.add(format("%d vertices", nvert));
	if (npoly > 0)
		s.add(format("%d polygons", npoly));
	return implode(s, ", ");
}

void ModeMesh::on_draw_post(Painter* p) {
	if (presentation_mode == PresentationMode::Vertices)
		session->drawing_helper->draw_data_points(p, multi_view->active_window, data->mesh->vertices, MultiViewType::MODEL_VERTEX, multi_view->hover);

	if (auto s = model_selection_description(data))
		draw_info(p, "selected: " + *s);
}

void ModeMesh::on_update_selection() {
	update_selection_vb();
}

void ModeMesh::update_vb() {
	VertexStagingBuffer vsb;
	for (auto& p: data->mesh->polygons)
		p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer, 1);
}


void ModeMesh::update_selection_vb() {
	VertexStagingBuffer vsb;
	for (auto& p: data->mesh->polygons)
		if (p.is_selected)
			p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer_selection, 1);
}


base::optional<Hover> ModeMesh::get_hover(MultiViewWindow* win, const vec2& m) const {
	if (presentation_mode == PresentationMode::Vertices) {
		base::optional<Hover> h;

		//float zmin = multi_view->view_port.radius * 2;
		for (const auto& [i, v]: enumerate(data->mesh->vertices)) {
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
		if (data->mesh->is_mouse_over(win, mat4::ID, m, tp, index, false))
			return Hover{MultiViewType::MODEL_POLYGON, index, tp};
	}
	return base::None;
}

void ModeMesh::select_in_rect(MultiViewWindow* win, const rect& r) {
	if (presentation_mode == PresentationMode::Vertices) {
		MultiView::select_points_in_rect(win, r, data->mesh->vertices);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		MultiView::select_points_in_rect(win, r, data->mesh->vertices);
		// vertices -> polygons
		for (auto& p: data->mesh->polygons) {
			p.is_selected = true;
			for (const auto& s: p.side)
				p.is_selected &= data->mesh->vertices[s.vertex].is_selected;
		}
	}
}

// also "self-consistency"...
base::optional<Box> ModeMesh::get_selection_box() const {
	if (presentation_mode == PresentationMode::Vertices) {
		// vertices -> polygons
		for (auto& p: data->mesh->polygons) {
			p.is_selected = true;
			for (const auto& s: p.side)
				p.is_selected &= data->mesh->vertices[s.vertex].is_selected;
		}

		return MultiView::points_get_selection_box(data->mesh->vertices);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		// polygons -> vertices
		for (auto& v: data->mesh->vertices)
			v.is_selected = false;
		for (auto& p: data->mesh->polygons)
			for (const auto& s: p.side)
				data->mesh->vertices[s.vertex].is_selected |= p.is_selected;

		return MultiView::points_get_selection_box(data->mesh->vertices);
	}
	return base::None;
}



void ModeMesh::on_command(const string& id) {
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}

void ModeMesh::copy() {
	temp_mesh->clear();
	temp_mesh->add(data->mesh->copy_geometry());
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
	if (key == xhui::KEY_CONTROL + xhui::KEY_C)
		copy();
	if (key == xhui::KEY_CONTROL + xhui::KEY_V)
		paste();
}

void ModeMesh::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeMesh::optimize_view() {
	multi_view->view_port.suggest_for_box(data->bounding_box());
}




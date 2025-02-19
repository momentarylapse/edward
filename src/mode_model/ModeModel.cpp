//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"
#include <Session.h>
#include <data/mesh/GeometryCube.h>
#include <helper/ResourceManager.h>
#include <lib/base/iter.h>
#include <lib/image/Painter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/controls/Toolbar.h>
#include <view/ActionController.h>
#include <view/MultiView.h>
#include "data/ModelMesh.h"
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <data/mesh/VertexStagingBuffer.h>
#include "ModeAddVertex.h"
#include "ModeAddPolygon.h"

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataModel(session);
	generic_data = data;
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	vertex_buffer_selection = new VertexBuffer("3f,3f,2f");
	material = create_material(session->resource_manager, White, 0.7f, 0.2f, Black);
	material_selection = create_material(session->resource_manager, Black.with_alpha(0.4f), 0.7f, 0.2f, Red, true);


	presentation_mode = PresentationMode::Polygons;
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter() {
	auto update = [this] {
		data->mesh->update_normals();
		update_vb();
		update_selection_vb();
	};

	auto win = session->win;
	auto tb = win->toolbar;
	tb->set_by_id("model-toolbar");

	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &data->mesh->vertices}
	};
	multi_view->out_selection_changed >> create_sink([this] {
		on_update_selection();
	});

	win->set_target("overlay-button-grid");
	win->add_control("Button", "V", 0, 1, "add-vertex");
	win->set_options("add-vertex", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "P", 0, 2, "add-polygon");
	win->set_options("add-polygon", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "V", 0, 3, "add-cube");
	win->set_options("add-cube", "height=50,width=50,noexpandx,ignorefocus");


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
		//session->set_mode(new ModeAddPolygon(this));
		PolygonMesh m = GeometryCube({200,0,0}, {20,0,0}, {0,20,0}, {0,0,20}, 2, 2, 2);
		data->paste_mesh(m, 0);
	}));

	data->out_changed >> create_sink(update);
	multi_view->view_port.radius = data->get_radius() * 2;
	update();
}

void ModeModel::on_leave() {
	data->out_changed.unsubscribe(this);
	for (int uid: event_ids)
		session->win->remove_event_handler(uid);
	event_ids.clear();
}

void ModeModel::set_presentation_mode(PresentationMode m) {
	presentation_mode = m;
	update_menu();
	session->win->request_redraw();
}

void ModeModel::update_menu() {

}



void ModeModel::on_prepare_scene(const RenderParams& params) {
}


void ModeModel::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
	auto dh = win->multi_view->session->drawing_helper;
	dh->clear(params, xhui::Theme::_default.background_low);


	if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer, material, 0);

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_selection, material_selection, 0);

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges or presentation_mode == PresentationMode::Polygons) {
		// backside
		// TODO draw_lines_with_color()
		dh->set_color(color(1, 0.35f, 0.35f, 0.35f));
		dh->set_line_width(1.5f);//scheme.LINE_WIDTH_THIN);

		Array<vec3> points;
		for (const auto& p: data->mesh->polygons) {
			if (vec3::dot(p.temp_normal, win->dir()) >= 0)
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
			if (vec3::dot(p.temp_normal, win->dir()) < 0)
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

void ModeModel::on_draw_post(Painter* p) {
	if (presentation_mode == PresentationMode::Vertices) {
		int _hover = -1;
		if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX)
			_hover = multi_view->hover->index;
		for (const auto& [i, v]: enumerate(data->mesh->vertices)) {
			p->set_color(v.is_selected ? Red : Blue);
			auto p1 = multi_view->active_window->project(v.pos);
			float r = 2;
			if (i == _hover)
				r = 4;
			p->draw_rect({p1.x - r,p1.x + r, p1.y - r,p1.y + r});
		}
	}

	p->set_color(White);
	if (auto s = model_selection_description(data))
		p->draw_str(p->area().p01() + vec2(30, -40), "selected: " + *s);
}

void ModeModel::on_update_selection() {
	//if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges) {
	for (auto& p: data->mesh->polygons) {
		p.is_selected = true;
		for (const auto& s: p.side)
			p.is_selected &= data->mesh->vertices[s.vertex].is_selected;
	}
	//}
	update_selection_vb();
}

void ModeModel::update_vb() {
	VertexStagingBuffer vsb;
	for (auto& p: data->mesh->polygons)
		p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer, 1);
}


void ModeModel::update_selection_vb() {
	VertexStagingBuffer vsb;
	for (auto& p: data->mesh->polygons)
		if (p.is_selected)
			p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer_selection, 1);
}


base::optional<Hover> ModeModel::get_hover(MultiViewWindow* win, const vec2& m) const {
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
	return base::None;
}

void ModeModel::on_command(const string& id) {
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}

void ModeModel::on_key_down(int key) {
	if (key == xhui::KEY_DELETE or key == xhui::KEY_BACKSPACE) {
		if (auto s = model_selection_description(data)) {
			data->delete_selection(data->get_selection(), presentation_mode == PresentationMode::Vertices);
			session->set_message("deleted: " + *s);
		} else {
			session->set_message("nothing selected");
		}
	}
}







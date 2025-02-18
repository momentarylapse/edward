//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"
#include <Session.h>
#include <helper/ResourceManager.h>
#include <lib/base/iter.h>
#include <lib/image/Painter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/controls/Toolbar.h>
#include <view/ActionController.h>
#include <view/MultiView.h>
#include "data/ModelMesh.h"
#include "data/ModelPolygon.h"
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include "ModeAddVertex.h"
#include "ModeAddPolygon.h"

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataModel(session);
	generic_data = data;
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	material = create_material(session->resource_manager, White, 0.7f, 0.2f, Black);

	presentation_mode = PresentationMode::Polygons;
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter() {
	auto update = [this] {
		data->mesh->update_normals();
		VertexStagingBuffer vsb;
		for (auto& p: data->mesh->polygon)
			p.add_to_vertex_buffer(data->mesh->vertex, vsb, 1);
		vsb.build(vertex_buffer, 1);
	};

	auto win = session->win;
	auto tb = win->toolbar;
	tb->set_by_id("model-toolbar");

	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &data->mesh->vertex}
	};

	win->set_target("overlay-button-grid");
	win->add_control("Button", "V", 0, 1, "add-vertex");
	win->set_options("add-vertex", "height=50,width=50,noexpandx,ignorefocus");
	win->add_control("Button", "P", 0, 2, "add-polygon");
	win->set_options("add-polygon", "height=50,width=50,noexpandx,ignorefocus");


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

	data->out_changed >> create_sink(update);
	multi_view->view_port.radius = data->getRadius() * 2;
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

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges) {
		dh->set_color(color(1, 0.5f, 0.5f, 0.5f));
		dh->set_line_width(3);//scheme.LINE_WIDTH_THIN);

		Array<vec3> points;
		for (const auto& p: data->mesh->polygon) {
			if (vec3::dot(p.temp_normal, win->dir()) < 0)
				for (int k=0; k<p.side.num; k++) {
					points.add(data->mesh->vertex[p.side[k].vertex].pos);
					points.add(data->mesh->vertex[p.side[(k + 1) % p.side.num].vertex].pos);
				}
		}
		dh->draw_lines(points, false);

	} else {
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer, material, 0);
	}


	multi_view->action_controller->draw(params, rvd);
}

void ModeModel::on_draw_post(Painter* p) {
	if (presentation_mode == PresentationMode::Vertices) {
		int _hover = -1;
		if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX)
			_hover = multi_view->hover->index;
		for (const auto& [i, v]: enumerate(data->mesh->vertex)) {
			p->set_color(v.is_selected ? Red : Blue);
			auto p1 = multi_view->active_window->project(v.pos);
			float r = 2;
			if (i == _hover)
				r = 4;
			p->draw_rect({p1.x - r,p1.x + r, p1.y - r,p1.y + r});
		}
	}
}

base::optional<Hover> ModeModel::get_hover(MultiViewWindow* win, const vec2& m) const {
	if (presentation_mode == PresentationMode::Vertices) {
		base::optional<Hover> h;

		//float zmin = multi_view->view_port.radius * 2;
		for (const auto& [i, v]: enumerate(data->mesh->vertex)) {
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







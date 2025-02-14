//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"
#include <Session.h>
#include <helper/ResourceManager.h>
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

#include "ModeAddVertexPolygon.h"

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataModel(session);
	generic_data = data;
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	material = create_material(session->resource_manager, White, 0.7f, 0.2f, Black);
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

	auto tb = session->win->toolbar;
	tb->set_by_id("model-toolbar");

	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) -> base::optional<Hover> {
		return base::None;
	};
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &data->mesh->vertex}
	};

	event_ids.add(session->win->event("add-entity", [this] {
		session->set_mode(new ModeAddVertexPolygon(this));
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


void ModeModel::on_prepare_scene(const RenderParams& params) {
}


void ModeModel::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
#ifdef USING_VULKAN
	auto cb = params.command_buffer;
	cb->clear(params.area, {xhui::Theme::_default.background_low}, 1.0);
	auto dh = win->multi_view->session->drawing_helper;


	auto shader = rvd.get_shader(material, 0, "default", "");
	auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, PrimitiveTopology::TRIANGLES, vertex_buffer);
	rd.apply(params);
	cb->draw(vertex_buffer);
#endif

	multi_view->action_controller->draw(params, rvd);
}

void ModeModel::on_draw_post(Painter* p) {
	for (auto& v: data->mesh->vertex) {
		p->set_color(v.is_selected ? Red : Blue);
		auto p1 = multi_view->active_window->project(v.pos);
		p->draw_rect({p1.x-2,p1.x+2, p1.y-2,p1.y+2});
	}
}






//
// Created by Michael Ankele on 2025-02-18.
//

#include "ModeAddPolygon.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddPolygon::ModeAddPolygon(ModeMesh* parent) :
	Mode(parent->session)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeAddPolygon::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

Mode* ModeAddPolygon::get_parent() {
	return mode_mesh;
}


void ModeAddPolygon::on_prepare_scene(const RenderParams& params) {
	mode_mesh->on_prepare_scene(params);
}

void ModeAddPolygon::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);

	session->drawing_helper->set_color(xhui::Theme::_default.text_label);
	session->drawing_helper->set_line_width(3);
	Array<vec3> points;
	for (int v: vertices)
		points.add(mode_mesh->data->mesh->vertices[v].pos);
	//points.add(multi_view->cursor_pos_3d());
	session->drawing_helper->draw_lines(points, true);
}

void ModeAddPolygon::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "click to add polygon");
}


void ModeAddPolygon::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeAddPolygon::on_left_button_down(const vec2& m) {
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX) {
		vertices.add(multi_view->hover->index);
	} else {
		const vec3 p = multi_view->cursor_pos_3d(m);
		mode_mesh->data->add_vertex(p, {0,0,0,0}, {0,0,0,0}, NORMAL_MODE_HARD);
		vertices.add(mode_mesh->data->mesh->vertices.num - 1);
	}

	if (vertices.num >= 3 and vertices.back() == vertices[0]) {
		vertices.pop();
		Array<vec3> sv;
		sv.resize(vertices.num);
		mode_mesh->data->add_polygon(vertices, 0);
		session->set_mode(mode_mesh);
	}

	session->win->request_redraw();
}


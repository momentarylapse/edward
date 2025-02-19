//
// Created by Michael Ankele on 2025-02-18.
//

#include "ModeAddPolygon.h"
#include "ModeModel.h"
#include "data/ModelMesh.h"
#include <Session.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddPolygon::ModeAddPolygon(ModeModel* parent) :
	Mode(parent->session)
{
	mode_model = parent;
	multi_view = mode_model->multi_view;
	generic_data = mode_model->generic_data;
}

void ModeAddPolygon::on_enter() {
	mode_model->set_presentation_mode(ModeModel::PresentationMode::Vertices);
}

Mode* ModeAddPolygon::get_parent() {
	return mode_model;
}


void ModeAddPolygon::on_prepare_scene(const RenderParams& params) {
	mode_model->on_prepare_scene(params);
}

void ModeAddPolygon::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_model->on_draw_win(params, win);

	session->drawing_helper->set_color(xhui::Theme::_default.text_label);
	session->drawing_helper->set_line_width(3);
	Array<vec3> points;
	for (int v: vertices)
		points.add(mode_model->data->mesh->vertices[v].pos);
	//points.add(multi_view->cursor_pos_3d());
	session->drawing_helper->draw_lines(points, true);
}

void ModeAddPolygon::on_draw_post(Painter* p) {
	mode_model->on_draw_post(p);

	p->set_color(xhui::Theme::_default.text_label);
	p->draw_str(p->area().p01() + vec2(30, -40), "click to add polygon");
}


void ModeAddPolygon::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_model);
	}
}

void ModeAddPolygon::on_left_button_down(const vec2& m) {
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX) {
		vertices.add(multi_view->hover->index);
	} else {
		const vec3 p = multi_view->cursor_pos_3d(m);
		mode_model->data->add_vertex(p, {0,0,0,0}, {0,0,0,0}, NORMAL_MODE_HARD);
		vertices.add(mode_model->data->mesh->vertices.num - 1);
	}

	if (vertices.num >= 3 and vertices.back() == vertices[0]) {
		vertices.pop();
		Array<vec3> sv;
		sv.resize(vertices.num);
		mode_model->data->add_polygon(vertices, 0);
		session->set_mode(mode_model);
	}

	session->win->request_redraw();
}


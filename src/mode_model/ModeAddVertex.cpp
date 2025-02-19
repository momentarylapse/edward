//
// Created by Michael Ankele on 2025-02-08.
//

#include "ModeAddVertex.h"
#include "ModeModel.h"
#include "data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddVertex::ModeAddVertex(ModeModel* parent) :
	Mode(parent->session)
{
	mode_model = parent;
	multi_view = mode_model->multi_view;
	generic_data = mode_model->generic_data;
}

void ModeAddVertex::on_enter() {
	mode_model->set_presentation_mode(ModeModel::PresentationMode::Vertices);
}

Mode* ModeAddVertex::get_parent() {
	return mode_model;
}


void ModeAddVertex::on_prepare_scene(const RenderParams& params) {
	mode_model->on_prepare_scene(params);
}

void ModeAddVertex::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_model->on_draw_win(params, win);
}

void ModeAddVertex::on_draw_post(Painter* p) {
	mode_model->on_draw_post(p);

	p->set_color(xhui::Theme::_default.text_label);
	p->draw_str(p->area().p01() + vec2(30, -40), "click to add vertices");
}


void ModeAddVertex::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_model);
	}
}

void ModeAddVertex::on_left_button_down(const vec2& m) {
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX)
		return;
	const vec3 p = multi_view->cursor_pos_3d(m);
	mode_model->data->add_vertex(p, {0,0,0,0}, {0,0,0,0}, NORMAL_MODE_HARD);
	session->win->request_redraw();
}



//
// Created by Michael Ankele on 2025-02-08.
//

#include "ModeAddVertexPolygon.h"
//#include "AddEntityPanel.h"
#include <Session.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

#include "ModeModel.h"
#include "data/ModelMesh.h"

ModeAddVertexPolygon::ModeAddVertexPolygon(ModeModel* parent) :
	Mode(parent->session)
{
	mode_model = parent;
	multi_view = mode_model->multi_view;
	generic_data = mode_model->generic_data;
}

void ModeAddVertexPolygon::on_enter() {
	session->set_message("XXXX");
	//mode_model->set_side_panel(new AddEntityPanel(mode_world));
}

Mode* ModeAddVertexPolygon::get_parent() {
	return mode_model;
}


void ModeAddVertexPolygon::on_prepare_scene(const RenderParams& params) {
	mode_model->on_prepare_scene(params);
}

void ModeAddVertexPolygon::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_model->on_draw_win(params, win);
}

void ModeAddVertexPolygon::on_draw_post(Painter* p) {
	mode_model->on_draw_post(p);

	p->set_color(xhui::Theme::_default.text_label);
	p->draw_str(p->area().p01() + vec2(30, -40), "click to add vertices");
}


void ModeAddVertexPolygon::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_model);
	}
}

void ModeAddVertexPolygon::on_left_button_down(const vec2& m) {
	const vec3 p = multi_view->cursor_pos_3d(m);
	mode_model->data->mesh->add_vertex(p, {0,0,0,0}, {0,0,0,0}, 0);
	session->win->request_redraw();
}



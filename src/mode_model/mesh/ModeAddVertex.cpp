//
// Created by Michael Ankele on 2025-02-08.
//

#include "ModeAddVertex.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddVertex::ModeAddVertex(ModeMesh* parent) :
	Mode(parent->session)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeAddVertex::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
}

Mode* ModeAddVertex::get_parent() {
	return mode_mesh;
}


void ModeAddVertex::on_prepare_scene(const RenderParams& params) {
	mode_mesh->on_prepare_scene(params);
}

void ModeAddVertex::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
}

void ModeAddVertex::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "click to add vertices");
}


void ModeAddVertex::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeAddVertex::on_left_button_down(const vec2& m) {
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_VERTEX)
		return;
	const vec3 p = multi_view->cursor_pos_3d(m);
	mode_mesh->data->add_vertex(p, {0,0,0,0}, {0,0,0,0}, NORMAL_MODE_HARD);
	session->win->request_redraw();
}



//
// Created by Michael Ankele on 2025-02-21.
//

#include "ModePaste.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModePaste::ModePaste(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	mode_mesh->temp_mesh->build(vertex_buffer.get());
	transformation = mat4::ID;
}

void ModePaste::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
	session->win->set_visible("overlay-button-grid-left", false);
}

void ModePaste::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);

	session->drawing_helper->draw_mesh(params, win->rvd(), transformation, vertex_buffer.get(), session->drawing_helper->material_creation);
}

void ModePaste::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "click to paste mesh");
}


void ModePaste::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModePaste::on_mouse_move(const vec2& m, const vec2& d) {
	const vec3 p = multi_view->cursor_pos_3d(m);
	transformation = mat4::translation(p - mode_mesh->temp_mesh->bounding_box().center());
	session->win->request_redraw();
}


void ModePaste::on_left_button_down(const vec2& m) {
	PolygonMesh mesh = *mode_mesh->temp_mesh;
	for (auto& v: mesh.vertices)
		v.pos = transformation * v.pos;
	mode_mesh->data->paste_mesh(mesh);
	session->set_mode(mode_mesh);
}

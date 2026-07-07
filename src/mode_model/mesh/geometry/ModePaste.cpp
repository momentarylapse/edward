//
// Created by Michael Ankele on 2025-02-21.
//

#include "ModePaste.h"
#include "ModeMeshGeometry.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <lib/ygraphics/graphics-impl.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/multiview/MultiView.h>

ModePaste::ModePaste(ModeMeshGeometry* parent) :
	SubMode(parent)
{
	mode_mesh = parent->mode_mesh;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
	parent->temp_mesh->build(vertex_buffer.get());
	transformation = mat4::ID;
}

void ModePaste::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModePaste::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	_parent->on_draw_win(params, win);

	session->drawing_helper->draw_mesh(params, win->rvd(), transformation, vertex_buffer.get(), session->drawing_helper->material_creation);
}

void ModePaste::on_draw_post(Painter* p) {
	_parent->on_draw_post(p);

	draw_info(p, "click to paste mesh");
}


void ModePaste::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		request_mode_end();
	}
}

void ModePaste::on_mouse_move(const vec2& m, const vec2& d) {
	const vec3 p = multi_view->cursor_pos_3d(m);
	transformation = mat4::translation(p - mode_mesh->mode_mesh_geometry->temp_mesh->bounding_box().value_or(Box::EMPTY).center());
	session->win->request_redraw();
}


void ModePaste::on_left_button_down(const vec2& m) {
	polymesh::Mesh mesh = *mode_mesh->mode_mesh_geometry->temp_mesh;
	for (auto& v: mesh.vertices)
		v.pos = transformation * v.pos;
	mode_mesh->data->paste_mesh(mesh);
	request_mode_end();
}

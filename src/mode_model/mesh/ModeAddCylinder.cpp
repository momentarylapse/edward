//
// Created by Michael Ankele on 2025-05-13.
//

#include "ModeAddCylinder.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/mesh/GeometryCylinder.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddCylinder::ModeAddCylinder(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}

void ModeAddCylinder::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	session->win->set_visible("overlay-button-grid-left", false);

	dialog = new xhui::Panel("xxx");
	dialog->from_resource("new_cylinder_dialog");
	session->win->embed("overlay-main-grid", 1, 0, dialog);

	edges = xhui::config.get_int("mesh.new_cylinder.edges", 32);
	rings = xhui::config.get_int("mesh.new_cylinder.rings", 1);
	round = xhui::config.get_bool("mesh.new_cylinder.round", false);
	dialog->set_int("edges", edges);
	dialog->set_int("rings", rings);
	dialog->check("round", round);
	dialog->event("edges", [this] {
		edges = dialog->get_int("edges");
	});
	dialog->event("rings", [this] {
		rings = dialog->get_int("rings");
	});
	dialog->event("round", [this] {
		round = dialog->is_checked("round");
	});
}

void ModeAddCylinder::on_leave() {
	session->win->unembed(dialog);
	xhui::config.set_int("mesh.new_cylinder.edges", edges);
	xhui::config.set_int("mesh.new_cylinder.rings", rings);
	xhui::config.set_bool("mesh.new_cylinder.round", round);
}


void ModeAddCylinder::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	dh->draw_mesh(params, win->rvd(), mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_MEDIUM);
	dh->set_z_test(false);
	if (points.num > 0)
		dh->draw_lines({points.back(), next_point});
	dh->set_z_test(true);
}

void ModeAddCylinder::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	if (points.num == 0)
		draw_info(p, "cylinder: place first point");
	else if (points.num == 1)
		draw_info(p, format("cylinder length: %s", multi_view->format_length((next_point - points[0]).length())));
	else if (points.num == 2)
		draw_info(p, format("clinder radius: %s", multi_view->format_length((next_point - points[1]).length())));
}


void ModeAddCylinder::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		doc->set_mode(mode_mesh);
	}
}

void ModeAddCylinder::on_mouse_move(const vec2& m, const vec2& d) {
	auto end_mode = round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT;
	if (points.num == 0) {
		next_point = multi_view->cursor_pos_3d(m);
		//mesh = GeometryCylinder(multi_view->cursor_pos_3d(m), {20,0,0}, {0,20,0}, {0,0,20}, slices[0], slices[1], slices[2]);
	} else if (points.num == 1) {
		next_point = multi_view->cursor_pos_3d(m);
		float r = multi_view->hover_window->pixel_to_size(50); // 50 px
		mesh = GeometryCylinder(points[0], next_point, r, rings, edges, end_mode);
	} else if (points.num == 2) {
		next_point = multi_view->hover_window->unproject(m, points[1]);
		float r = (next_point - points[1]).length();
		mesh = GeometryCylinder(points[0], points[1], r, rings, edges, end_mode);
	}

	mesh.build(vertex_buffer.get());

	session->win->request_redraw();
}


void ModeAddCylinder::on_left_button_down(const vec2& m) {
	if (points.num >= 2) {
		mode_mesh->data->paste_mesh(mesh, 0);
		doc->set_mode(mode_mesh);
	} else {
		points.add(next_point);
	}

	session->win->request_redraw();
}

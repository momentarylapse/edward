//
// Created by Michael Ankele on 2025-02-23.
//

#include "ModeAddSphere.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <data/mesh/GeometryBall.h>
#include <data/mesh/GeometrySphere.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddSphere::ModeAddSphere(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}

void ModeAddSphere::on_enter() {
	//mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	session->win->set_visible("overlay-button-grid-left", false);

	dialog = new xhui::Panel("xxx");
	dialog->from_resource("new_ball_dialog");
	session->win->embed("overlay-main-grid", 1, 0, dialog);

	slices[0] = xhui::config.get_int("mesh.new_sphere.slices_x", 8);
	slices[1] = xhui::config.get_int("mesh.new_sphere.slices_y", 16);
	complexity = xhui::config.get_int("mesh.new_sphere.complexity", 8);
	type = (Type)xhui::config.get_int("mesh.new_sphere.type", (int)Type::Ball);
	dialog->set_int("x", slices[0]);
	dialog->set_int("y", slices[1]);
	dialog->set_int("complexity", complexity);
	dialog->check("type:ball", type == Type::Ball);
	dialog->check("type:sphere", type == Type::Sphere);

	dialog->event("x", [this] {
		slices[0] = dialog->get_int("x");
	});
	dialog->event("y", [this] {
		slices[1] = dialog->get_int("y");
	});
	dialog->event("complexity", [this] {
		complexity = dialog->get_int("complexity");
	});
	auto on_type = [this] {
		if (dialog->is_checked("type:ball"))
			type = Type::Ball;
		else
			type = Type::Sphere;
		dialog->enable("x", type == Type::Ball);
		dialog->enable("y", type == Type::Ball);
		dialog->enable("complexity", type == Type::Sphere);
	};
	dialog->event("type:ball", on_type);
	dialog->event("type:sphere", on_type);

	on_type();

	center = base::None;
	radius = multi_view->hover_window->pixel_to_size(50);
	update_mesh();
}

void ModeAddSphere::on_leave() {
	session->win->unembed(dialog);
	xhui::config.set_int("mesh.new_sphere.slices_x", slices[0]);
	xhui::config.set_int("mesh.new_sphere.slices_y", slices[1]);
	xhui::config.set_int("mesh.new_sphere.complexity", complexity);
	xhui::config.set_int("mesh.new_sphere.type", (int)type);
}

void ModeAddSphere::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	dh->draw_mesh(params, win->rvd(), mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_MEDIUM);
	dh->set_z_test(false);
	if (center)
		dh->draw_lines({*center, next_point});
	dh->set_z_test(true);
}

void ModeAddSphere::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);
	if (center)
		draw_info(p, format("sphere: radius %s", multi_view->format_length(radius)));
	else
		draw_info(p, "sphere: place center");
}


void ModeAddSphere::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeAddSphere::update_mesh() {
	vec3 p = center.value_or(next_point);
	if (type == Type::Ball)
		mesh = GeometryBall(p, radius, slices[0], slices[1]);
	else
		mesh = GeometrySphere(p, radius, complexity);
	mesh.build(vertex_buffer.get());

	session->win->request_redraw();
}


void ModeAddSphere::on_mouse_move(const vec2& m, const vec2& d) {
	if (center) {
		next_point = multi_view->hover_window->unproject(m, *center);
		radius = (next_point - *center).length();
	} else {
		next_point = multi_view->cursor_pos_3d(m);
	}
	update_mesh();
}


void ModeAddSphere::on_left_button_down(const vec2& m) {
	if (center) {
		mode_mesh->data->paste_mesh(mesh, 0);
		session->set_mode(mode_mesh);
	} else {
		center = multi_view->cursor_pos_3d(m);
	}

	session->win->request_redraw();
}
//
// Created by Michael Ankele on 2025-02-23.
//

#include "ModeAddSphere.h"
#include "ModeMeshGeometry.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/polymesh/create/Ball.h>
#include <lib/polymesh/create/Sphere.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/helper/LineHelper.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddSphere::ModeAddSphere(ModeMeshGeometry* parent) :
	SubMode(parent)
{
	mode_mesh = parent->mode_mesh;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}

void ModeAddSphere::on_enter() {
	//mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	dialog = new xhui::Panel("xxx");
	dialog->from_resource("new_ball_dialog");
	set_overlay_panel(dialog);

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
	dialog->event("cancel", [this] {
		request_mode_end();
	});

	on_type();

	center = base::None;
	radius = multi_view->hover_window->pixel_to_size(50);
	update_mesh();
}

void ModeAddSphere::on_leave() {
	set_overlay_panel(nullptr);
	xhui::config.set_int("mesh.new_sphere.slices_x", slices[0]);
	xhui::config.set_int("mesh.new_sphere.slices_y", slices[1]);
	xhui::config.set_int("mesh.new_sphere.complexity", complexity);
	xhui::config.set_int("mesh.new_sphere.type", (int)type);
}

void ModeAddSphere::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	_parent->on_draw_win(params, win);
	auto lh = session->line_helper;

	session->drawing_helper->draw_mesh(params, win->rvd(), mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);

	lh->set_color(DrawingHelper::COLOR_X);
	lh->set_line_width(DrawingHelper::LINE_MEDIUM);
	lh->set_z_test(false);
	if (center)
		lh->draw_lines({*center, next_point}, false);
	lh->set_z_test(true);
}

void ModeAddSphere::on_draw_post(Painter* p) {
	_parent->on_draw_post(p);
	if (center)
		draw_info(p, format("sphere: radius %s", multi_view->format_length(radius)));
	else
		draw_info(p, "sphere: place center");
}


void ModeAddSphere::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		request_mode_end();
	}
}

void ModeAddSphere::update_mesh() {
	vec3 p = center.value_or(next_point);
	if (type == Type::Ball)
		mesh = polymesh::create_ball(p, radius, slices[0], slices[1]);
	else
		mesh = polymesh::create_sphere(p, radius, complexity);
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
		if (mode_mesh->data->editing_mesh == mode_mesh->data->phys_mesh) {
			polymesh::Mesh m2;
			m2.add_vertex(*center);
			m2.spheres.add({0, radius});
			mode_mesh->data->paste_mesh(m2, 0);
		} else {
			mode_mesh->data->paste_mesh(mesh, 0);
		}
		request_mode_end();
	} else {
		center = multi_view->cursor_pos_3d(m);
	}

	session->win->request_redraw();
}
//
// Created by Michael Ankele on 2025-02-24.
//

#include "ModeAddPlatonic.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <data/mesh/GeometryPlatonic.h>
#include <data/mesh/GeometryTeapot.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddPlatonic::ModeAddPlatonic(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new VertexBuffer("3f,3f,2f");
}

void ModeAddPlatonic::on_enter() {
	//mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	session->win->set_visible("overlay-button-grid-left", false);

	dialog = new xhui::Panel("xxx");
	dialog->from_resource("new-platonic-dialog");
	session->win->embed("overlay-main-grid", 1, 0, dialog);

	complexity = xhui::config.get_int("mesh.new_platonic.complexity", 4);
	type = (Type)xhui::config.get_int("mesh.new_platonic.type", (int)Type::Teapot);
	dialog->set_int("complexity", complexity);
	dialog->check("type:tetrahedron", type == Type::Tetrahedron);
	dialog->check("type:cube", type == Type::Cube);
	dialog->check("type:octahedron", type == Type::Octahedron);
	dialog->check("type:dodecahedron", type == Type::Dodecahedron);
	dialog->check("type:icosahedron", type == Type::Icosahedron);
	dialog->check("type:teapot", type == Type::Teapot);

	dialog->event("complexity", [this] {
		complexity = dialog->get_int("complexity");
	});
	auto on_type = [this] {
		if (dialog->is_checked("type:tetrahedron"))
			type = Type::Tetrahedron;
		else if (dialog->is_checked("type:cube"))
			type = Type::Cube;
		else if (dialog->is_checked("type:octahedron"))
			type = Type::Octahedron;
		else if (dialog->is_checked("type:dodecahedron"))
			type = Type::Dodecahedron;
		else if (dialog->is_checked("type:icosahedron"))
			type = Type::Icosahedron;
		else
			type = Type::Teapot;
		dialog->enable("complexity", type == Type::Teapot);
	};
	dialog->event("type:tetrahedron", on_type);
	dialog->event("type:cube", on_type);
	dialog->event("type:octahedron", on_type);
	dialog->event("type:dodecahedron", on_type);
	dialog->event("type:icosahedron", on_type);
	dialog->event("type:teapot", on_type);

	on_type();

	center = {0,0,0};
	center_selected = false;
	radius = 10;
	update_mesh();
}

void ModeAddPlatonic::on_leave() {
	session->win->unembed(dialog);
	xhui::config.set_int("mesh.new_platonic.complexity", complexity);
	xhui::config.set_int("mesh.new_platonic.type", (int)type);
}

void ModeAddPlatonic::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);

	session->drawing_helper->draw_mesh(params, win->rvd(), mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);
}

void ModeAddPlatonic::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);
	if (center_selected)
		draw_info(p, format("sphere: radius %s", multi_view->format_length(radius)));
	else
		draw_info(p, "sphere: place center");
}


void ModeAddPlatonic::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeAddPlatonic::update_mesh() {
	if (type == Type::Tetrahedron)
		mesh = GeometryPlatonic(center, radius, 4);
	else if (type == Type::Cube)
		mesh = GeometryPlatonic(center, radius, 6); // meh
	else if (type == Type::Octahedron)
		mesh = GeometryPlatonic(center, radius, 8);
	else if (type == Type::Dodecahedron)
		mesh = GeometryPlatonic(center, radius, 12);
	else if (type == Type::Icosahedron)
		mesh = GeometryPlatonic(center, radius, 20);
	else
		mesh = GeometryTeapot(center, radius, complexity);

	mesh.build(vertex_buffer.get());

	session->win->request_redraw();
}


void ModeAddPlatonic::on_mouse_move(const vec2& m, const vec2& d) {
	if (center_selected) {
		vec3 pos2 = multi_view->cursor_pos_3d(m);
		radius = (pos2 - center).length();
	} else {
		center = multi_view->cursor_pos_3d(m);
	}
	update_mesh();
}


void ModeAddPlatonic::on_left_button_down(const vec2& m) {
	if (center_selected) {
		mode_mesh->data->paste_mesh(mesh, 0);
		session->set_mode(mode_mesh);
	} else {
		center = multi_view->cursor_pos_3d(m);
		center_selected = true;
	}

	session->win->request_redraw();
}
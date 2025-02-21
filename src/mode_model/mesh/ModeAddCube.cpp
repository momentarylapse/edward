//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeAddCube.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <data/mesh/GeometryCube.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddCube::ModeAddCube(ModeMesh* parent) :
	Mode(parent->session)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new VertexBuffer("3f,3f,2f");
}

void ModeAddCube::on_enter() {
	//mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

Mode* ModeAddCube::get_parent() {
	return mode_mesh;
}


void ModeAddCube::on_prepare_scene(const RenderParams& params) {
	mode_mesh->on_prepare_scene(params);
}

void ModeAddCube::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);

	session->drawing_helper->draw_mesh(params, win->rvd, mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);
}

void ModeAddCube::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	if (points.num == 0)
		draw_info(p, "cube: place first corner point");
	else if (points.num == 1)
		draw_info(p, format("cube base area: %s x %s", multi_view->format_length(length[0].length()), multi_view->format_length(length[1].length())));
	else if (points.num == 2)
		draw_info(p, format("cube: %s x %s x %s", multi_view->format_length(length[0].length()), multi_view->format_length(length[1].length()), multi_view->format_length(length[2].length())));
}


void ModeAddCube::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

bool ModeAddCube::set_dpos3(const vec2& m) {
	vec3 n = vec3::cross(length[0], length[1]).normalized();
	vec3 dpos = multi_view->cursor_pos_3d(m) - points[1];
	float min_thick = 10 / multi_view->hover_window->zoom(); // 10 px


	if (fabs(vec3::dot(multi_view->hover_window->edit_frame() * vec3::EZ, n)) > 0.90f) {
		// cursor in cube plane -> use radius
		length[2] = n * multi_view->maybe_snap_f(max(dpos.length(), min_thick)) * 2;
		if (vec3::dot(multi_view->hover_window->direction(), n) < 0)
			length[2] = -length[2];
		return true;
	}

	length[2] = multi_view->maybe_snap_v(n * vec3::dot(n, dpos)) * 2;
	if (length[2].length() < min_thick)
		length[2] = n * min_thick;
	return false;
}

void ModeAddCube::on_mouse_move(const vec2& m, const vec2& d) {
	if (points.num == 0) {
		mesh = GeometryCube(multi_view->cursor_pos_3d(m), {20,0,0}, {0,20,0}, {0,0,20}, 2, 2, 2);
	} else if (points.num == 1) {
		vec3 pos2 = multi_view->cursor_pos_3d(m);
		const mat3 frame = multi_view->hover_window->edit_frame();
		vec3 dir1 = frame * vec3::EX;
		vec3 dir2 = frame * vec3::EY;
		length[0] = dir1 * vec3::dot(dir1, pos2 - points[0]);
		length[1] = dir2 * vec3::dot(dir2, pos2 - points[0]);
		float min_thick = 10 / multi_view->hover_window->zoom(); // 10 px
		vec3 n = vec3::cross(length[0], length[1]).normalized();
		length[2] = n * min_thick;
		mesh = GeometryCube(points[0] - length[2]/2, length[0], length[1], length[2], 2, 2, 2);
	} else if (points.num == 2) {
		set_dpos3(m);
		mesh = GeometryCube(points[0] - length[2]/2, length[0], length[1], length[2], 2, 2, 2);
	}


	mesh.build(vertex_buffer.get());

	session->win->request_redraw();
}


void ModeAddCube::on_left_button_down(const vec2& m) {
	if (points.num >= 2) {
		mode_mesh->data->paste_mesh(mesh, 0);
		session->set_mode(mode_mesh);
	} else {
		points.add(multi_view->cursor_pos_3d(m));
	}

	session->win->request_redraw();
}
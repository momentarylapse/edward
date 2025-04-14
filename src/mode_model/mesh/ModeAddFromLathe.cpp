//
// Created by Michael Ankele on 2025-04-14.
//

#include "ModeAddFromLathe.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <data/mesh/GeometryCube.h>
#include <lib/os/msg.h>
#include <lib/xhui/config.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

ModeAddFromLathe::ModeAddFromLathe(ModeMesh* parent) :
	Mode(parent->session)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;

	vertex_buffer = new VertexBuffer("3f,3f,2f");
}

void ModeAddFromLathe::on_enter() {
	//mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Vertices);
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);

	session->win->set_visible("overlay-button-grid-left", false);

	dialog = new xhui::Panel("xxx");
	dialog->from_source(R"foodelim(
Dialog new-lathe-dialog "Lathe" allow-root width=200 noexpandx
	Grid ? ""
		Grid ? "" vertical class=card
			Label header "Lathe" big bold center
			Group ncdt_slides "Stripes"
				Grid ? ""
					SpinButton stripes "" range=1: expandx
		---|
		Label ? "" expandy ignorehover

)foodelim");
	//dialog->from_resource("new_cube_dialog");
	session->win->embed("overlay-main-grid", 1, 0, dialog);

	slices = xhui::config.get_int("mesh.new_lathe.stripes", 32);
	dialog->set_int("stripes", slices);
	dialog->event("stripes", [this] {
		slices = dialog->get_int("stripes");
	});
}

void ModeAddFromLathe::on_leave() {
	session->win->unembed(dialog);
	xhui::config.set_int("mesh.new_lathe.stripes", slices);
}


Mode* ModeAddFromLathe::get_parent() {
	return mode_mesh;
}


void ModeAddFromLathe::on_prepare_scene(const RenderParams& params) {
	mode_mesh->on_prepare_scene(params);
}

void ModeAddFromLathe::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);

	session->drawing_helper->draw_mesh(params, win->rvd, mat4::ID, vertex_buffer.get(), session->drawing_helper->material_creation);

	session->drawing_helper->set_color(White);
	session->drawing_helper->set_line_width(3);
	float r = 200 / multi_view->hover_window->zoom(); // 200 px
	if (center) {
		vec3 a = axis.value_or(preview_axis);
		session->drawing_helper->draw_lines({*center - a * r, *center + a * r});
	}

	if (contour.num >= 2)
		session->drawing_helper->draw_lines(contour);
}

void ModeAddFromLathe::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	if (!center)
		draw_info(p, "lathe: place center");
	else if (!axis)
		draw_info(p, "lathe: place 2nd axis point");
	else
		draw_info(p, "lathe: place contour points");
		//draw_info(p, format("cube: %s x %s x %s", multi_view->format_length(length[0].length()), multi_view->format_length(length[1].length()), multi_view->format_length(length[2].length())));
}


void ModeAddFromLathe::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
	if (key == xhui::KEY_RETURN and contour.num > 0) {
		mode_mesh->data->paste_mesh(mesh, 0);
		session->set_mode(mode_mesh);
	}
}

vec3 ModeAddFromLathe::suggest_axis(const vec3& p2) const {
	vec3 d = (p2 - *center);
	if (fabs(d.x) > fabs(d.y) and (fabs(d.x) > fabs(d.z)))
		return vec3::EX;
	if (fabs(d.y) > fabs(d.z))
		return vec3::EY;
	return vec3::EZ;
}

void ModeAddFromLathe::create_turned_mesh() {
	mesh.clear();

	vec3 e1 = axis->ortho();
	vec3 e2 = vec3::cross(*axis, e1);

	for (const vec3& _p: contour) {
		vec3 p0 = _p - *center;
		float x0 = vec3::dot(e1, p0);
		float y0 = vec3::dot(e2, p0);
		float r = sqrt(x0*x0 + y0*y0);
		float z = vec3::dot(*axis, p0);
		for (int i=0; i<slices; i++) {
			float theta = (float)i / (float)slices * 2 * pi;
			vec3 p = *center + z * *axis + (cos(theta) * e1 + sin(theta) * e2) * r;
			mesh.add_vertex(p);
		}
	}

	for (int k=0; k<contour.num-1; k++)
		for (int i=0; i<slices; i++) {
			int ii = (i+1) % slices;
			int a = k*slices + i;
			int b = k*slices + ii;
			int c = (k+1)*slices + i;
			int d = (k+1)*slices + ii;
			mesh.add_polygon_auto_texture({a, b, d, c});
		}
}


void ModeAddFromLathe::on_mouse_move(const vec2& m, const vec2& d) {
	if (center and !axis) {
		vec3 pos2 = multi_view->cursor_pos_3d(m);
		preview_axis = suggest_axis(pos2);
	}
	session->win->request_redraw();
}


void ModeAddFromLathe::on_left_button_down(const vec2& m) {
	if (!center) {
		center = multi_view->cursor_pos_3d(m);
	} else if (!axis) {
		axis = suggest_axis(multi_view->cursor_pos_3d(m));
	} else {
		contour.add(multi_view->cursor_pos_3d(m));
		create_turned_mesh();
		mesh.build(vertex_buffer.get());
	}

	session->win->request_redraw();
}
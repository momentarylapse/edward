//
// Created by Michael Ankele on 2025-05-18.
//

#include "ModeExtrudePolygons.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/algo.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>


MeshEdit prepare_extrude(const PolygonMesh& mesh, const Data::Selection& sel, float distance) {
	MeshEdit ed;
	for (const auto& [i, p]: enumerate(mesh.polygons))
		if (sel[MultiViewType::MODEL_POLYGON].contains(i)) {
			ed.delete_polygon(i);
			Polygon pp = p;
			Array<int> new_vertices;
			for (int k=0; k<p.side.num; k++) {
				int v0 = p.side[k].vertex;
				MeshVertex vv = mesh.vertices[v0];
				vv.pos += distance * p.get_normal(mesh.vertices);
				int v1 = ed.add_vertex(vv);
				new_vertices.add(v1);
				pp.side[k].vertex = v1;
			}
			for (int k=0; k<p.side.num; k++) {
				Polygon ppp;
				ppp.side.resize(4);
				ppp.side[0].vertex = p.side[k].vertex;
				ppp.side[1].vertex = p.side[(k+1) % p.side.num].vertex;
				ppp.side[2].vertex = new_vertices[(k+1) % p.side.num];
				ppp.side[3].vertex = new_vertices[k];
				ed.add_polygon(ppp);
			}
			ed.add_polygon(pp);
		}
	return ed;
}

Array<vec3> mesh_edit_to_lines(const PolygonMesh& mesh, const MeshEdit& ed) {
	Array<vec3> points;
	for (const auto& p: ed.new_polygons) {
		for (int k=0; k<p.side.num; k++) {
			int v0 = p.side[k].vertex;
			int v1 = p.side[(k+1) % p.side.num].vertex;
			points.add((v0 >= 0) ? mesh.vertices[v0].pos : ed.new_vertices[-(v0+1)].pos);
			points.add((v1 >= 0) ? mesh.vertices[v1].pos : ed.new_vertices[-(v1+1)].pos);
		}
	}
	return points;
}


ModeExtrudePolygons::ModeExtrudePolygons(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeExtrudePolygons::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_action(false);
	session->win->set_visible("overlay-button-grid-left", false);
}

void ModeExtrudePolygons::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	auto ed = prepare_extrude(*mode_mesh->data->editing_mesh, mode_mesh->data->get_selection(), 20);

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_THICK);
	dh->set_z_test(false);
	auto points = mesh_edit_to_lines(*mode_mesh->data->editing_mesh, ed);
	dh->draw_lines(points, false);
	dh->set_z_test(true);
}

void ModeExtrudePolygons::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "extrude...");
}


void ModeExtrudePolygons::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
	if (key == xhui::KEY_RETURN) {
		auto ed = prepare_extrude(*mode_mesh->data->editing_mesh, mode_mesh->data->get_selection(), 20);
		mode_mesh->data->edit_mesh(ed);
		session->set_mode(mode_mesh);
	}
}

void ModeExtrudePolygons::on_left_button_down(const vec2& m) {
	session->win->request_redraw();
}

void ModeExtrudePolygons::on_mouse_move(const vec2& m, const vec2& d) {
	session->win->request_redraw();
}

//
// Created by Michael Ankele on 2025-05-18.
//

#include "ModeBevelEdges.h"
#include "ModeMesh.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/algo.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>


struct BevelInfo {
	struct Cap {
		int index;
		vec3 p0;
		Array<vec3> dirs;
		base::map<int,int> next, prev;
		base::map<int,vec3> dir_next, dir_prev;
	};
	Array<Cap> caps;
	Array<Edge> edges;

	Array<vec3> to_lines(float r) const {
		Array<vec3> lines;
		for (const auto& c: caps) {
			for (int k=0; k<c.dirs.num; k++) {
				lines.add(c.p0 + c.dirs[k] * r);
				lines.add(c.p0 + c.dirs[(k+1)%c.dirs.num] * r);
			}
		}

		for (const auto& e: edges) {
			auto c0 = base::find_by_element(caps, &Cap::index, e.index[0]);
			auto c1 = base::find_by_element(caps, &Cap::index, e.index[1]);
			lines.add(c0->p0 + r*c0->dir_next[e.index[1]]);
			lines.add(c1->p0 + r*c1->dir_prev[e.index[0]]);
			lines.add(c1->p0 + r*c1->dir_next[e.index[0]]);
			lines.add(c0->p0 + r*c0->dir_prev[e.index[1]]);
		}
		return lines;
	}
};

BevelInfo prepare_bevel(const PolygonMesh& mesh, const Data::Selection& sel) {
	BevelInfo b;
	auto edges = mesh.edges();
	for (const auto& [i, v]: enumerate(mesh.vertices))
		if (sel[MultiViewType::MODEL_VERTEX].contains(i)) {
			const vec3 p0 = mesh.vertices[i].pos;
			BevelInfo::Cap cap;
			cap.index = i;
			cap.p0 = p0;
			for (const auto& c: mesh.get_polygons_around_vertex(i)) {
				//i = c.polygon->side[c.side].vertex;
				int i_next = c.polygon->next_vertex(i);
				int i_prev = c.polygon->previous_vertex(i);
				const vec3 dir_next = (mesh.vertices[i_next].pos - p0).normalized();
				const vec3 dir_prev = (mesh.vertices[i_prev].pos - p0).normalized();
				if (sel[MultiViewType::MODEL_VERTEX].contains(i_next) and sel[MultiViewType::MODEL_VERTEX].contains(i_prev)) {
					cap.dirs.add((dir_next + dir_prev).normalized());
					cap.dir_next.set(i_prev, (dir_next + dir_prev).normalized());
					cap.dir_prev.set(i_next, (dir_next + dir_prev).normalized());
				} else if (!sel[MultiViewType::MODEL_VERTEX].contains(i_next)) {
					cap.dirs.add(dir_next);
					cap.dir_next.set(i_prev, dir_next);
					cap.dir_prev.set(mesh.next_edge_at_vertex(i, i_next), dir_next);
				}
				cap.next.set(i_prev, i_next);
				cap.prev.set(i_next, i_prev);
			}
			b.caps.add(cap);
		}

	for (const auto& e: edges)
		if (sel[MultiViewType::MODEL_VERTEX].contains(e.index[0]) and sel[MultiViewType::MODEL_VERTEX].contains(e.index[1]))
			b.edges.add(e);
	return b;
}



ModeBevelEdges::ModeBevelEdges(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
}

void ModeBevelEdges::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Edges);
	multi_view->set_allow_action(false);
	session->win->set_visible("overlay-button-grid-left", false);
}

void ModeBevelEdges::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_mesh->on_draw_win(params, win);
	auto dh = session->drawing_helper;

	auto b = prepare_bevel(*mode_mesh->data->mesh, mode_mesh->data->get_selection());

	dh->set_color(DrawingHelper::COLOR_X);
	dh->set_line_width(DrawingHelper::LINE_THICK);
	dh->set_z_test(false);
	Array<vec3> points = b.to_lines(10);
	dh->draw_lines(points, false);
	dh->set_z_test(true);
}

void ModeBevelEdges::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);

	draw_info(p, "bevel...");
}


void ModeBevelEdges::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_mesh);
	}
}

void ModeBevelEdges::on_left_button_down(const vec2& m) {
	session->win->request_redraw();
}

void ModeBevelEdges::on_mouse_move(const vec2& m, const vec2& d) {
	session->win->request_redraw();
}


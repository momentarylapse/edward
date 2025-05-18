//
// Created by michi on 18.05.25.
//

#include "MeshBevelEdges.h"
#include <Session.h>
#include <data/mesh/Polygon.h>
#include <data/mesh/PolygonMesh.h>
#include <lib/base/iter.h>
#include <lib/base/algo.h>

#include "lib/os/msg.h"


struct BevelInfo {
	struct Cap {
		int index;
		vec3 p0;
		Array<vec3> dirs;
		Array<int> new_vertices;
		base::map<int,int> next_dir_no, prev_dir_no;
		//base::map<int,vec3> dir_next, dir_prev;
	};
	Array<Cap> caps;
	Array<Edge> edges;

	/*Array<vec3> to_lines(float r) const {
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
	}*/
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
					cap.dirs.add(dir_next + dir_prev);
					cap.next_dir_no.set(i_prev, cap.dirs.num - 1);
					cap.prev_dir_no.set(i_next, cap.dirs.num - 1);
				} else if (!sel[MultiViewType::MODEL_VERTEX].contains(i_next)) {
					cap.dirs.add(dir_next);
					cap.next_dir_no.set(i_prev, cap.dirs.num - 1);
					cap.prev_dir_no.set(mesh.next_edge_at_vertex(i, i_next), cap.dirs.num - 1);
				}
				//cap.next.set(i_prev, i_next);
				//cap.prev.set(i_next, i_prev);
			}
			b.caps.add(cap);
		}

	for (const auto& e: edges)
		if (sel[MultiViewType::MODEL_VERTEX].contains(e.index[0]) and sel[MultiViewType::MODEL_VERTEX].contains(e.index[1]))
			b.edges.add(e);
	return b;
}


MeshEdit mesh_prepare_bevel_edges(const PolygonMesh& mesh, const Data::Selection& sel, float distance) {
	auto b = prepare_bevel(mesh, sel);
	MeshEdit ed;

	for (auto& c: b.caps)
		for (const auto& d: c.dirs)
			c.new_vertices.add(ed.add_vertex(MeshVertex{c.p0 + d * distance}));

	for (const auto& c: b.caps)
		if (c.new_vertices.num >= 3) {
			Polygon p;
			p.side.resize(c.new_vertices.num);
			for (int k=0; k<c.new_vertices.num; k++)
				p.side[k].vertex = c.new_vertices[k];
			ed.add_polygon(p);
		}

	for (const auto& e: b.edges) {
		auto c0 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[0]);
		auto c1 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[1]);
		Polygon p;
		p.side.resize(4);
		p.side[0].vertex = c0->new_vertices[c0->next_dir_no[e.index[1]]];
		p.side[1].vertex = c1->new_vertices[c1->prev_dir_no[e.index[0]]];
		p.side[2].vertex = c1->new_vertices[c1->next_dir_no[e.index[0]]];
		p.side[3].vertex = c0->new_vertices[c0->prev_dir_no[e.index[1]]];
		ed.add_polygon(p);
	}
	return ed;
}



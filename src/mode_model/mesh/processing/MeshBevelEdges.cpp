//
// Created by michi on 18.05.25.
//

#include "MeshBevelEdges.h"
#include <Session.h>
#include <data/mesh/Polygon.h>
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/MeshEdit.h>
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
		base::map<int,int> edge_to_dir;
	};
	Array<Cap> caps;
	Array<Edge> edges;
};

bool mesh_get_polygons_and_edges_around_vertex(const PolygonMesh& mesh, int vertex, Array<PolygonCorner>& out_corners, Array<Edge>& out_edges) {
	out_corners.clear();
	out_edges.clear();
	base::set<int> cornders_used;

	const auto corners = mesh.get_polygons_around_vertex(vertex);
	if (corners.num == 0)
		return false;

	out_corners.add(corners[0]);
	cornders_used.add(0);

	while (true) {
		const auto c = out_corners.back();
		const auto e = c.polygon->get_side_edge_in(c.side);
		out_edges.add(e);

		bool found = false;
		for (const auto& [i, cc]: enumerate(corners))
			if (cc.polygon->get_side_edge_out(cc.side) == e) {
				if (i == 0)
					// cycle complete
					return true;

				if (cornders_used.contains(i)) {
					// error... bad loop
					return false;
				}

				out_corners.add(cc);
				cornders_used.add(i);
				found = true;
			}

		if (!found)
			return false;
	}
}

BevelInfo prepare_bevel(const PolygonMesh& mesh, const Array<Edge>& edges, const Data::Selection& sel) {
	const auto& selv = sel[MultiViewType::MODEL_VERTEX];
	BevelInfo b;

	for (const auto& [i, v]: enumerate(mesh.vertices))
		if (selv.contains(i)) {
			const vec3 p0 = mesh.vertices[i].pos;
			Array<PolygonCorner> vcorners;
			Array<Edge> vedges;
			if (!mesh_get_polygons_and_edges_around_vertex(mesh, i, vcorners, vedges))
				continue;

			BevelInfo::Cap cap;
			cap.index = i;
			cap.p0 = p0;

			for (const auto& [ci, c]: enumerate(vcorners)) {
				int i_next = c.polygon->next_vertex(i);
				int i_prev = c.polygon->previous_vertex(i);
				vec3 a = mesh.vertices[i_prev].pos;
				vec3 b = mesh.vertices[i_next].pos;

				// on polygon
				cap.dirs.add(((a - p0).normalized() + (b - p0).normalized()).normalized());

				// along edge
				cap.dirs.add((a - p0).normalized());
				cap.edge_to_dir.set(edges.find(vedges[ci]), cap.dirs.num - 1);
			}
			b.caps.add(cap);
		}

	for (const auto& [i, v]: enumerate(mesh.vertices))
		if (selv.contains(i) and false) {
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
				} else if (!selv.contains(i_next)) {
					cap.dirs.add(dir_next);
					cap.next_dir_no.set(i_prev, cap.dirs.num - 1);
					cap.prev_dir_no.set(mesh.next_edge_at_vertex(i, i_next), cap.dirs.num - 1);
				}
				//cap.next.set(i_prev, i_next);
				//cap.prev.set(i_next, i_prev);
			}
			b.caps.add(cap);
		}

	/*for (const auto& e: edges)
		if (sele.contains(edges.find(e)))
			b.edges.add(e);*/
	return b;
}


MeshEdit mesh_prepare_bevel_edges(const PolygonMesh& mesh, const Data::Selection& sel, float radius) {
	auto edges = mesh.edges();
	auto b = prepare_bevel(mesh, edges, sel);
	MeshEdit ed;
	const auto& sele = sel[MultiViewType::MODEL_EDGE];

	for (auto& c: b.caps)
		for (const auto& d: c.dirs)
			c.new_vertices.add(ed.add_vertex(MeshVertex{c.p0 + d * radius}));

	for (const auto& c: b.caps)
		if (c.new_vertices.num >= 3) {
			Polygon p;
			p.side.resize(c.new_vertices.num);
			for (int k=0; k<c.new_vertices.num; k++)
				p.side[k].vertex = c.new_vertices[k];
			ed.add_polygon(p);
		}

	for (const auto& [i, e]: enumerate(edges)) {
		if (sele.contains(i)) {
			auto c0 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[0]);
			auto c1 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[1]);
			int d0 = c0->edge_to_dir[i];
			int d1 = c1->edge_to_dir[i];
			Polygon p;
			p.side.resize(4);
			p.side[0].vertex = c0->new_vertices[loop(d0 - 1, 0, c0->dirs.num)];
			p.side[1].vertex = c1->new_vertices[loop(d1 + 1, 0, c1->dirs.num)];
			p.side[2].vertex = c1->new_vertices[loop(d1 - 1, 0, c1->dirs.num)];
			p.side[3].vertex = c0->new_vertices[loop(d0 + 1, 0, c0->dirs.num)];
			ed.add_polygon(p);
		}
	}

	/*for (const auto& e: b.edges) {
		auto c0 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[0]);
		auto c1 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[1]);
		Polygon p;
		p.side.resize(4);
		p.side[0].vertex = c0->new_vertices[c0->next_dir_no[e.index[1]]];
		p.side[1].vertex = c1->new_vertices[c1->prev_dir_no[e.index[0]]];
		p.side[2].vertex = c1->new_vertices[c1->next_dir_no[e.index[0]]];
		p.side[3].vertex = c0->new_vertices[c0->prev_dir_no[e.index[1]]];
		ed.add_polygon(p);
	}*/
	return ed;
}



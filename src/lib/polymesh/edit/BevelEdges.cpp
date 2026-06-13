//
// Created by michi on 18.05.25.
//

#include "BevelEdges.h"
#include <lib/polymesh/Polygon.h>
#include <lib/polymesh/PolygonMesh.h>
#include <lib/polymesh/MeshEdit.h>
#include <lib/base/iter.h>
#include <lib/base/algo.h>
#include <lib/base/map.h>

#include "lib/os/msg.h"


namespace polymesh {

struct BevelInfo {
	struct Cap {
		int index;
		vec3 p0;
		Array<vec3> edge_dirs;
		Array<vec3> polygon_dirs;
		Array<int> new_vertices;
		Array<int> new_vertices_e;
		Array<int> new_vertices_p; // between edges #i-1 / #i
		//base::map<int,int> next_dir_no, prev_dir_no;
		//base::map<int,vec3> dir_next, dir_prev;
		base::map<int,int> edge_to_dir;
		Array<PolygonCorner> corners;
		Array<int> edges;
	};
	Array<Cap> caps;
	Array<Edge> edges;
};

bool mesh_get_polygons_and_edges_around_vertex(const PolygonMesh& mesh, int vertex, Array<PolygonCorner>& out_corners, Array<Edge>& out_edges) {
	out_corners.clear();
	out_edges.clear();
	base::set<int> corners_used;

	const auto corners = mesh.get_polygons_around_vertex(vertex);
	if (corners.num == 0)
		return false;

	out_corners.add(corners[0]);
	corners_used.add(0);

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

				if (corners_used.contains(i)) {
					// error... bad loop
					return false;
				}

				out_corners.add(cc);
				corners_used.add(i);
				found = true;
			}

		if (!found)
			return false;
	}
}

BevelInfo prepare_bevel(const PolygonMesh& mesh, const Array<Edge>& edges, const base::set<int>& selv) {
	BevelInfo bi;

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
			cap.corners = vcorners;

			for (const auto& [ci, c]: enumerate(vcorners)) {
				int i_next = c.polygon->next_vertex(i);
				int i_prev = c.polygon->previous_vertex(i);
				vec3 a = mesh.vertices[i_prev].pos;
				vec3 b = mesh.vertices[i_next].pos;

				// on polygon
				cap.polygon_dirs.add(((a - p0).normalized() + (b - p0).normalized()).normalized());

				// along edge
				int edge = edges.find(vedges[ci]);
				cap.edge_dirs.add((a - p0).normalized());
				cap.edge_to_dir.set(edge, ci);
				cap.edges.add(edge);
			}
			bi.caps.add(cap);
		}
	return bi;
}

MeshEdit bevel_edges(const PolygonMesh& mesh, const base::set<int>& selv, const base::set<int>& sele, float radius) {
	auto edges = mesh.edges();
	auto b = prepare_bevel(mesh, edges, selv);
	MeshEdit ed;

	// new vertices
	for (auto& c: b.caps) {
		c.new_vertices_e.resize(c.edges.num);
		c.new_vertices_p.resize(c.edges.num);
		for (auto&& [i, d]: enumerate(c.edge_dirs)) {
			c.new_vertices_e[i] = c.new_vertices_p[i] = -1; // meh
			if (!sele.contains(c.edges[i])) {
				int v = ed.add_vertex(MeshVertex{c.p0 + d * radius});
				c.new_vertices_e[i] = v;
				c.new_vertices.add(v);
			}
			if (sele.contains(c.edges[i]) and sele.contains(c.edges[(i + c.edges.num - 1) % c.edges.num])) {
				int v = ed.add_vertex(MeshVertex{c.p0 + c.polygon_dirs[i] * radius});
				c.new_vertices_p[i] = v;
				c.new_vertices.add(v);
			}
		}
	}

	// new cap polygons
	for (const auto& c: b.caps) {
		if (c.new_vertices.num >= 3) {
			Polygon p;
			p.side.resize(c.new_vertices.num);
			for (int k=0; k<c.new_vertices.num; k++)
				p.side[k].vertex = c.new_vertices[k];
			ed.add_polygon(p);
		}
	}

	// new edge polygons
	for (const auto& [i, e]: enumerate(edges)) {
		if (sele.contains(i)) {
			auto c0 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[0]);
			auto c1 = base::find_by_element(b.caps, &BevelInfo::Cap::index, e.index[1]);
			int d0 = c0->edge_to_dir[i];
			int d0_prev = loop(d0 - 1, 0, c0->edges.num);
			int d0_next = loop(d0 + 1, 0, c0->edges.num);
			int d1 = c1->edge_to_dir[i];
			int d1_prev = loop(d1 - 1, 0, c1->edges.num);
			int d1_next = loop(d1 + 1, 0, c1->edges.num);
			Polygon p;
			p.side.resize(4);
			if (sele.contains(c0->edges[d0_prev]))
				p.side[0].vertex = c0->new_vertices_p[d0];
			else
				p.side[0].vertex = c0->new_vertices_e[d0_prev];
			if (sele.contains(c1->edges[d1_next]))
				p.side[1].vertex = c1->new_vertices_p[d1_next];
			else
				p.side[1].vertex = c1->new_vertices_e[d1_next];
			if (sele.contains(c1->edges[d1_prev]))
				p.side[2].vertex = c1->new_vertices_p[d1];
			else
				p.side[2].vertex = c1->new_vertices_e[d1_prev];
			if (sele.contains(c0->edges[d0_next]))
				p.side[3].vertex = c0->new_vertices_p[d0_next];
			else
				p.side[3].vertex = c0->new_vertices_e[d0_next];
			ed.add_polygon(p);
		}
	}

	// remap/shrink polygons
	for (const auto& [i, p]: enumerate(mesh.polygons)) {
		bool changed = false;
		for (const auto& s: p.side)
			if (selv.contains(s.vertex))
				changed = true;
		if (!changed)
			continue;
		Polygon pp;
		for (const auto& [si, s]: enumerate(p.side))
			if (selv.contains(s.vertex)) {
				auto c = base::find_by_element(b.caps, &BevelInfo::Cap::index, s.vertex);
				int e0 = edges.find(p.get_side_edge_in(si));
				int e1 = edges.find(p.get_side_edge_out(si));
				if (sele.contains(e0) and sele.contains(e1)) {
					pp.side.add(PolygonSide{c->new_vertices_p[c->edge_to_dir[e0]]});
				} else if (sele.contains(e0)) {
					pp.side.add(PolygonSide{c->new_vertices_e[c->edge_to_dir[e1]]});
				} else if (sele.contains(e1)) {
					pp.side.add(PolygonSide{c->new_vertices_e[c->edge_to_dir[e0]]});
				} else {
					pp.side.add(PolygonSide{c->new_vertices_e[c->edge_to_dir[e0]]});
					pp.side.add(PolygonSide{c->new_vertices_e[c->edge_to_dir[e1]]});
				}

			} else {
				pp.side.add(s);
			}
		ed.delete_polygon(i);
		ed.add_polygon(pp);
	}

	// delete beveled corner vertices
	for (auto& c: b.caps)
		ed.delete_vertex(c.index);

	return ed;
}

}

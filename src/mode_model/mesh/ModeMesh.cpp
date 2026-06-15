//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeMesh.h"
#include "geometry/ModeMeshGeometry.h"
#include "normals/ModeMeshNormals.h"
#include "sculpt/ModeMeshSculpt.h"
#include "uv/ModeMeshUV.h"
#include "material/ModeMeshMaterial.h"
#include "../ModeModel.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/polymesh/create/Cylinder.h>
#include <lib/polymesh/create/Sphere.h>
#include <helper/ResourceManager.h>
#include <lib/base/iter.h>
#include <lib/image/Painter.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <view/ActionController.h>
#include <view/MultiView.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/DocumentSession.h>
#include <lib/ygraphics/graphics-impl.h>
#include <cmath>
#include <lib/polymesh/MeshEdit.h>

yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeMesh::ModeMesh(ModeModel* parent) : SubMode(parent) {
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
	vertex_buffer_physical = new ygfx::VertexBuffer("3f,3f,2f");
	vertex_buffer_selection = new ygfx::VertexBuffer("3f,3f,2f");
	vertex_buffer_hover = new ygfx::VertexBuffer("3f,3f,2f");
	material_physical = create_material(session->ctx, Black.with_alpha(0.4f), 0.7f, 0.2f, color(1,1,1,0.4f).srgb_to_linear(), true);
	material_selection = create_material(session->ctx, Black.with_alpha(0.4f), 0.7f, 0.2f, Red, true);
	material_hover = create_material(session->ctx, Black.with_alpha(0.4f), 0.7f, 0.2f, White, true);

	current_material = 0;
	current_texture_level = 0;

	presentation_mode = PresentationMode::Polygons;

	mode_mesh_geometry = new ModeMeshGeometry(this);
	mode_mesh_material = new ModeMeshMaterial(this);
	mode_mesh_normals = new ModeMeshNormals(this);
	mode_mesh_sculpt = new ModeMeshSculpt(this);
	mode_mesh_uv = new ModeMeshUV(this);
}

ModeMesh::~ModeMesh() = default;

void ModeMesh::set_current_material(int index) {
	if (index == current_material or index < 0)
		return;
	current_material = index;
	set_current_texture_level(0);
	out_current_material_changed();
}

void ModeMesh::set_current_texture_level(int index) {
	if (index == current_texture_level or index < 0)
		return;
	current_texture_level = index;
	out_texture_level_changed();
}


void ModeMesh::set_edit_mesh(ModelMesh* mesh) {
	data->editing_mesh = mesh;
	multi_view->data_sets = {
		{MultiViewType::MODEL_VERTEX, &mesh->vertices},
		{MultiViewType::MODEL_POLYGON, &mesh->polygons},
	};
	data->out_mesh_edited(polymesh::MeshEdit{});
	on_update_menu();
	update_vb();
}

void ModeMesh::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});
}

void ModeMesh::on_connect_events_rec() {
	doc->event("mesh-visible0", [this] {
		set_edit_mesh(data->mesh.get());
	});
	doc->event("mesh-physical", [this] {
		set_edit_mesh(data->phys_mesh.get());
	});

	doc->event("mode_model_vertex", [this] {
		set_presentation_mode(PresentationMode::Vertices);
	});
	doc->event("mode_model_edge", [this] {
		set_presentation_mode(PresentationMode::Edges);
	});
	doc->event("mode_model_polygon", [this] {
		set_presentation_mode(PresentationMode::Polygons);
	});
	doc->event("mode_model_surface", [this] {
		set_presentation_mode(PresentationMode::Surfaces);
	});
}

void ModeMesh::on_leave_rec() {
	doc->out_changed.unsubscribe(this);
}

void ModeMesh::on_enter() {
	auto update = [this] {
		normals_dirty = true;
		update_vb();
		update_selection_vb();
		out_redraw();
	};
	xhui::run_later(0.01f, [this] {
		doc->set_mode(mode_mesh_geometry.get());
	});

	auto win = session->win;

	win->enable("mode-mesh-uv", false);
	win->enable("mode-mesh-paint", false);

	multi_view->set_show_grid(true);
	set_edit_mesh(data->mesh.get());
	multi_view->out_selection_changed >> create_sink([this] {
		on_update_selection();
	});

	data->out_changed >> create_sink(update);
	data->out_mesh_edited >> create_data_sink<polymesh::MeshEdit>([this] (const polymesh::MeshEdit&) {
		on_update_topology();
	});

	on_update_topology();
	data->editing_mesh->update_normals();
	normals_dirty = false;
	update();

	xhui::run_repeated(1.0f, [this] {
		if (normals_dirty) {
			data->editing_mesh->update_normals();
			update_edge_info();
			update_vb();
			update_selection_vb();
			out_redraw();
			normals_dirty = false;
		}
	});
}

void ModeMesh::on_connect_events() {
}


void ModeMesh::on_leave() {
	data->out_changed.unsubscribe(this);
	set_overlay_panel(nullptr);
}

void ModeMesh::on_update_topology() {
	edges_cached = data->editing_mesh->edges();
	update_edge_info();
}

void ModeMesh::update_edge_info() {
	edge_infos.resize(edges_cached.num);
	for (auto& ei: edge_infos)
		ei.polygons[0] = ei.polygons[1] = -1;

	for (const auto& [i, p]: enumerate(data->editing_mesh->polygons)) {
		for (const auto& [k, e]: enumerate(p.get_edges())) {
			auto& ei = edge_infos[edges_cached.find(e)];
			if (ei.polygons[0] < 0) {
				ei.polygons[1] = i;
				ei.sides[1] = k;
				ei.normal = p.temp_normal;
			} else {
				ei.polygons[0] = i;
				ei.sides[0] = k;
				ei.normal = (ei.normal + p.temp_normal).normalized();
			}
		}
	}
}



void ModeMesh::set_presentation_mode(PresentationMode m) {
	presentation_mode = m;
	make_selection_consistent(multi_view->selection);
	multi_view->out_selection_changed();
	on_update_menu();
	out_redraw();
}

void ModeMesh::on_update_menu() {
	_parent->on_update_menu();
}

void ModeMesh::update_menu_presentation_mode() {
	auto win = session->win;
	win->check("mode_model_vertex", presentation_mode == PresentationMode::Vertices);
	win->check("mode_model_edge", presentation_mode == PresentationMode::Edges);
	win->check("mode_model_polygon", presentation_mode == PresentationMode::Polygons);
	win->check("mode_model_surface", presentation_mode == PresentationMode::Surfaces);
}

void ModeMesh::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low.srgb_to_linear()});
}

void ModeMesh::draw_polygons(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;

	for (int i=0; i<vertex_buffers.num; i++)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffers[i], data->materials[i], 0);
}

void ModeMesh::draw_edges(const yrenderer::RenderParams& params, MultiViewWindow* win, const base::set<int>& sel) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;
	// unselected (colored by normal)
	Array<vec3> points;
	Array<color> colors;
	for (const auto& [i, e]: enumerate(edges_cached))
		if (!sel.contains(i)) {
			points.add(data->editing_mesh->vertices[e.index[0]].pos);
			points.add(data->editing_mesh->vertices[e.index[1]].pos);
			float f = (vec3::dot(edge_infos[i].normal, win->direction()) + 1) * 0.5f;
			const auto c = color::mix(color(1, 0.6f, 0.6f, 0.6f), color(1, 0.25f, 0.25f, 0.25f), f).srgb_to_linear();
			colors.add(c);
			colors.add(c);
		}
	dh->set_line_width(1.5f);//scheme.LINE_WIDTH_THIN);
	dh->draw_lines_colored(points, colors, false);

	// selected
	points.clear();
	for (const auto& [i, e]: enumerate(edges_cached))
		if (sel.contains(i)) {
			points.add(data->editing_mesh->vertices[e.index[0]].pos);
			points.add(data->editing_mesh->vertices[e.index[1]].pos);
		}
	dh->set_color(Red);
	dh->set_line_width(2);//scheme.LINE_WIDTH_THIN);
	dh->draw_lines(points, false);

	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_EDGE) {
		points.clear();
		const auto& e = edges_cached[multi_view->hover->index];
		points.add(data->editing_mesh->vertices[e.index[0]].pos);
		points.add(data->editing_mesh->vertices[e.index[1]].pos);
		dh->set_color(White);
		dh->set_line_width(4);//scheme.LINE_WIDTH_THIN);
		dh->draw_lines(points, false);
	}
}

void ModeMesh::draw_mesh(const yrenderer::RenderParams& params, MultiViewWindow* win, bool with_selection) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;
	const auto& selv = multi_view->selection[MultiViewType::MODEL_VERTEX];
	const auto& sele = multi_view->selection[MultiViewType::MODEL_EDGE];
	const auto& selp = multi_view->selection[MultiViewType::MODEL_POLYGON];

	if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces or data->editing_mesh == data->phys_mesh.get())
		draw_polygons(params, win);

	if (data->editing_mesh == data->phys_mesh.get())
		// spheres & cylinders
			dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_physical, material_physical, 0);

	if (with_selection) {
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_selection, material_selection, 0);

		Array<ygfx::Vertex1> buf;
		if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_POLYGON)
			data->editing_mesh->polygons[multi_view->hover->index].add_to_vertex_buffer(data->editing_mesh->vertices, buf);
		vertex_buffer_hover->update(buf);
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_hover, material_hover, 0);
	}

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges or presentation_mode == PresentationMode::Polygons)
		draw_edges(params, win, with_selection ? sele : base::set<int>());
}

void ModeMesh::on_update_selection() {
	update_selection_vb();
}

void ModeMesh::update_vb() {
	vertex_buffers.resize(data->materials.num);
	for (int i=0; i<vertex_buffers.num; i++) {
		if (!vertex_buffers[i])
			vertex_buffers[i] = new ygfx::VertexBuffer("3f,3f,2f");

		Array<ygfx::Vertex1> buf;
		for (auto& p: data->mesh->polygons)
			if (p.material == i)
				p.add_to_vertex_buffer(data->mesh->vertices, buf);
		vertex_buffers[i]->update(buf);
	}

	polymesh::Mesh m;
	m.vertices = data->phys_mesh->vertices;
	m.polygons = data->phys_mesh->polygons;
	for (const auto& b: data->editing_mesh->spheres)
		m.add(polymesh::create_sphere(data->editing_mesh->vertices[b.index].pos, b.radius, 8));
	for (const auto& c: data->editing_mesh->cylinders)
		m.add(polymesh::create_cylinder(data->editing_mesh->vertices[c.index[0]].pos, data->editing_mesh->vertices[c.index[1]].pos, c.radius, 1, 32));
	m.build(vertex_buffer_physical);
}


void ModeMesh::update_selection_vb() {
	Array<ygfx::Vertex1> buf;
	const auto& selp = multi_view->selection[MultiViewType::MODEL_POLYGON];
	for (auto&& [i, p]: enumerate(data->editing_mesh->polygons))
		if (selp.contains(i))
			p.add_to_vertex_buffer(data->editing_mesh->vertices, buf);
	// TODO spheres/cylinders
	vertex_buffer_selection->update(buf);
}

vec2 line_closest_point2d(const vec2& a, const vec2& b, const vec2& p) {
	vec2 dir = (b - a).normalized();
	return a + dir * vec2::dot(p - a, dir);
}

float vec2_factor_between(const vec2& a, const vec2& b, const vec2& p) {
	if (fabs(b.x - a.x) > fabs(b.y - a.y))
		return (p.x - a.x) / (b.x - a.x);
	return (p.y - a.y) / (b.y - a.y);
}

base::optional<Hover> ModeMesh::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;
	if (presentation_mode == PresentationMode::Vertices) {

		//float zmin = multi_view->view_port.radius * 2;
		for (const auto& [i, v]: enumerate(data->editing_mesh->vertices)) {
			const auto pp = win->project(v.pos);
			if (pp.z <= 0 or pp.z >= 1)
				continue;
			if ((pp.xy() - m).length_fuzzy() > 10)
				continue;
			h = {MultiViewType::MODEL_VERTEX, i, v.pos};
		}
	} else if (presentation_mode == PresentationMode::Edges) {
		float zmax = 1;
		for (const auto& [i, e]: enumerate(edges_cached)) {
			const auto pp0 = win->project(data->editing_mesh->vertices[e.index[0]].pos);
			const auto pp1 = win->project(data->editing_mesh->vertices[e.index[1]].pos);
			if (pp0.z <= 0 or pp0.z >= 1 or pp1.z <= 0 or pp1.z >= 1)
				continue;
			vec2 xx = line_closest_point2d(pp0.xy(), pp1.xy(), m);
			if ((xx - m).length_fuzzy() > 10)
				continue;
			float f = vec2_factor_between(pp0.xy(), pp1.xy(), xx);
			if (f < 0 or f > 1)
				continue;
			const auto p = (1 - f) * data->editing_mesh->vertices[e.index[0]].pos + f * data->editing_mesh->vertices[e.index[1]].pos;
			const auto pp = win->project(p);
			if (pp.z > zmax)
				continue;
			zmax = pp.z;
			h = {MultiViewType::MODEL_EDGE, i, p};
		}
	} else if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces) {
		vec3 tp;
		int index;
		if (data->editing_mesh->is_mouse_over(win, mat4::ID, m, tp, index, false))
			return Hover{MultiViewType::MODEL_POLYGON, index, tp};
	}
	return h;
}

void selection_edges_from_vertices(base::set<int>& sele, const base::set<int>& selv, const Array<polymesh::Edge>& edges) {
	sele.clear();
	for (const auto& [i, e]: enumerate(edges)) {
		if (selv.contains(e.index[0]) and selv.contains(e.index[1]))
			sele.add(i);
	}
}

void selection_polygons_from_vertices(base::set<int>& selp, const base::set<int>& selv, const polymesh::Mesh& mesh) {
	selp.clear();
	for (const auto& [i, p]: enumerate(mesh.polygons)) {
		bool is_selected = true;
		for (const auto& s: p.side)
			is_selected &= selv.contains(s.vertex);
		if (is_selected)
			selp.add(i);
	}
}

void selection_vertices_from_edges(base::set<int>& selv, const base::set<int>& sele, const Array<polymesh::Edge>& edges) {
	selv.clear();
	for (const auto& [i,e]: enumerate(edges))
		if (sele.contains(i)) {
			selv.add(e.index[0]);
			selv.add(e.index[1]);
		}
}

void selection_polygons_from_edges(base::set<int>& selp, const base::set<int>& sele, const polymesh::Mesh& mesh, const Array<polymesh::Edge>& edges) {
	selp.clear();
	for (const auto& [i,p]: enumerate(mesh.polygons)) {
		bool is_selected = true;
		for (const auto& e: p.get_edges())
			if (!sele.contains(edges.find(e)))
				is_selected = false;
		if (is_selected)
			selp.add(i);
	}
}

void selection_vertices_from_polygons(base::set<int>& selv, const base::set<int>& selp, const polymesh::Mesh& mesh) {
	selv.clear();
	for (const auto& [i,p]: enumerate(mesh.polygons))
		if (selp.contains(i))
			for (const auto& s: p.side)
				selv.add(s.vertex);
}

void selection_edges_from_polygons(base::set<int>& sele, const base::set<int>& selp, const polymesh::Mesh& mesh, const Array<polymesh::Edge>& edges) {
	sele.clear();
	for (const auto& [i,p]: enumerate(mesh.polygons))
		if (selp.contains(i))
			for (const auto& e: p.get_edges())
				sele.add(edges.find(e));
}

Selection ModeMesh::select_in_rect(MultiViewWindow* win, const rect& r) {
	Selection sel;
	sel.add({MultiViewType::MODEL_VERTEX, {}});
	sel.add({MultiViewType::MODEL_EDGE, {}});
	sel.add({MultiViewType::MODEL_POLYGON, {}});
	sel.add({MultiViewType::MODEL_BALL, {}});
	sel.add({MultiViewType::MODEL_CYLINDER, {}});

	auto selv = sel[MultiViewType::MODEL_VERTEX] = MultiView::select_points_in_rect(win, r, data->editing_mesh->vertices);
	if (presentation_mode == PresentationMode::Vertices) {
		sel[MultiViewType::MODEL_VERTEX] = selv;
	} else if (presentation_mode == PresentationMode::Edges) {
		selection_edges_from_vertices(sel[MultiViewType::MODEL_EDGE], selv, edges_cached);
	} else if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces) {
		selection_polygons_from_vertices(sel[MultiViewType::MODEL_POLYGON], selv, *data->editing_mesh);
	}
	return sel;
}

void ModeMesh::make_selection_consistent(Selection &sel) const {
	auto& selv = sel[MultiViewType::MODEL_VERTEX];
	auto& sele = sel[MultiViewType::MODEL_EDGE];
	auto& selp = sel[MultiViewType::MODEL_POLYGON];
	if (presentation_mode == PresentationMode::Vertices) {
		selection_edges_from_vertices(sele, selv, edges_cached);
		selection_polygons_from_vertices(selp, selv, *data->editing_mesh);
		/*for (auto& b: data->editing_mesh->spheres)
			b.is_selected = data->editing_mesh->vertices[b.index].is_selected;
		for (auto& c: data->editing_mesh->cylinders)
			c.is_selected = data->editing_mesh->vertices[c.index[0]].is_selected and data->editing_mesh->vertices[c.index[1]].is_selected;*/
	} else if (presentation_mode == PresentationMode::Edges) {
		selection_vertices_from_edges(selv, sele, edges_cached);
		selection_polygons_from_edges(selp, sele, *data->editing_mesh, edges_cached);
	} else if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces) {
		if (presentation_mode == PresentationMode::Surfaces) {
			// grow selection along connected polygons
			int n0;
			do {
				n0 = selp.num;
				selection_vertices_from_polygons(selv, selp, *data->editing_mesh);
				for (const auto& [i, p]: enumerate(data->editing_mesh->polygons)) {
					for (const auto& s: p.side)
						if (selv.contains(s.vertex))
							selp.add(i);
				}
			} while (selp.num > n0);
		}
		selection_vertices_from_polygons(selv, selp, *data->editing_mesh);
		selection_edges_from_polygons(sele, selp, *data->editing_mesh, edges_cached);
	}
}

base::optional<Box> ModeMesh::get_selection_box(const Selection& sel) const {
	return MultiView::points_get_selection_box(data->editing_mesh->vertices, sel[MultiViewType::MODEL_VERTEX]);
}





//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeMesh.h"
#include "ModeAddVertex.h"
#include "ModeAddPolygon.h"
#include "ModeAddCube.h"
#include "ModeAddSphere.h"
#include "ModeAddPlatonic.h"
#include "ModeAddFromLathe.h"
#include "ModeAddCylinder.h"
#include "ModePaste.h"
#include "ModeBevelEdges.h"
#include "ModeExtrudePolygons.h"
#include "ModeMeshSculpt.h"
#include "material/ModeMeshMaterial.h"
#include "../ModeModel.h"
#include "action/ActionModelMoveSelection.h"
#include "../data/ModelMesh.h"
#include "material/dialog/ModelMaterialSelectionDialog.h"
#include <Session.h>
#include <lib/mesh/GeometryCylinder.h>
#include <lib/mesh/GeometrySphere.h>
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
#include <lib/mesh/VertexStagingBuffer.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/MenuBar.h>
#include <cmath>

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

	temp_mesh = new ModelMesh();
	current_material = 0;
	current_texture_level = 0;

	presentation_mode = PresentationMode::Polygons;

	mode_mesh_material = new ModeMeshMaterial(this);
	mode_mesh_sculpt = new ModeMeshSculpt(this);
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
	on_update_menu();
	update_vb();
}

void ModeMesh::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});
}

void ModeMesh::on_connect_events_rec() {
	doc->event("mode_model_mesh", [this] {
		doc->set_mode(this);
	});
	doc->event("mode_model_deform", [this] {
		doc->set_mode(mode_mesh_sculpt.get());
	});
	doc->event("mode_model_materials", [this] {
		doc->set_mode(mode_mesh_material.get());
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

void ModeMesh::on_set_menu() {
	_parent->on_set_menu();
	auto menu = xhui::create_resource_menu("menu_model");
	session->win->menu_bar->set_menu(menu);
}

class MeshOpButtons : public xhui::Panel {
public:
	explicit MeshOpButtons(MultiView* multi_view) : xhui::Panel("mesh-op-buttons") {
		from_source(R"foodelim(
Dialog mesh-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button mouse-action 'T' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-vertex 'V' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-polygon 'P' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-cube 'Q' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-sphere 'S' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-platonic 'P' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-from-lathe 'L' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-cylinder 'C' height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");

		event("mouse-action", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			const auto mode = ac->action_mode();
			if (mode == MouseActionMode::MOVE) {
				ac->set_action_mode(MouseActionMode::ROTATE);
				set_options("mouse-action", "image=rf-rotate");
			} else if (mode == MouseActionMode::ROTATE) {
				ac->set_action_mode(MouseActionMode::SCALE);
				set_options("mouse-action", "image=rf-scale");
			} else if (mode == MouseActionMode::SCALE) {
				ac->set_action_mode(MouseActionMode::MOVE);
				set_options("mouse-action", "image=rf-translate");
			}
			set_string("mouse-action", multi_view->action_controller->action_name().sub(0, 1).upper());
		});
	}
};

void ModeMesh::on_enter() {
	auto update = [this] {
		normals_dirty = true;
		update_vb();
		update_selection_vb();
		session->win->request_redraw();
	};

	auto win = session->win;

	auto menu_bar = (xhui::MenuBar*)win->get_control("menu");
	auto menu = xhui::create_resource_menu("menu_model");
	menu_bar->set_menu(menu);

	win->enable("mode_model_texture_coord", false);
	win->enable("mode_model_paint", false);

	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		return select_in_rect(win, r);
	};
	multi_view->f_make_selection_consistent = [this] (Data::Selection& sel) {
		return make_selection_consistent(sel);
	};
	multi_view->f_get_selection_box = [this] (const Data::Selection& sel) {
		return get_selection_box(sel);
	};
	multi_view->f_create_action = [this] {
		return new ActionModelMoveSelection(data->editing_mesh, multi_view->selection);
	};
	set_edit_mesh(data->mesh.get());
	multi_view->out_selection_changed >> create_sink([this] {
		on_update_selection();
	});

	set_overlay_panel(new MeshOpButtons(multi_view));

	data->out_changed >> create_sink(update);
	data->out_topology_changed >> create_sink([this] {
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
			session->win->request_redraw();
			normals_dirty = false;
		}
	});
}

void ModeMesh::on_connect_events() {
	doc->event("mesh-visible0", [this] {
		set_edit_mesh(data->mesh.get());
	});
	doc->event("mesh-physical", [this] {
		set_edit_mesh(data->phys_mesh.get());
	});


	doc->event("add-vertex", [this] {
		doc->set_mode(new ModeAddVertex(this));
	});
	doc->event("add-polygon", [this] {
		doc->set_mode(new ModeAddPolygon(this));
	});
	doc->event("add-cube", [this] {
		doc->set_mode(new ModeAddCube(this));
	});
	doc->event("add-sphere", [this] {
		doc->set_mode(new ModeAddSphere(this));
	});
	doc->event("add-platonic", [this] {
		doc->set_mode(new ModeAddPlatonic(this));
	});
	doc->event("add-from-lathe", [this] {
		doc->set_mode(new ModeAddFromLathe(this));
	});
	doc->event("add-cylinder", [this] {
		doc->set_mode(new ModeAddCylinder(this));
	});
	doc->event("normal_this_hard", [this] {
		const auto& sel = multi_view->selection[MultiViewType::MODEL_POLYGON];
		for (auto&& [i, p]: enumerate(data->mesh->polygons))
			if (sel.contains(i)) {
				p.smooth_group = -1;
				p.normal_dirty = true;
			}
		data->mesh->update_normals();
		data->out_changed();
	});
	doc->event("normal_this_smooth", [this] {
		const auto& sel = multi_view->selection[MultiViewType::MODEL_POLYGON];
		for (auto&& [i, p]: enumerate(data->mesh->polygons))
			if (sel.contains(i)) {
				p.smooth_group = 42;
				p.normal_dirty = true;
			}
		data->mesh->update_normals();
		data->out_changed();
	});
	doc->event("choose_material", [this] {
		ModelMaterialSelectionDialog::ask(this).then([this] (int material) {
			data->apply_material(multi_view->selection, material);
		});
	});
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
	session->win->request_redraw();
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


void ModeMesh::on_prepare_scene(const yrenderer::RenderParams& params) {
}

void ModeMesh::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low.srgb_to_linear()});
}

void ModeMesh::draw_polygons(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;

	for (int i=0; i<vertex_buffers.num; i++)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffers[i], materials[i], 0);
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

void ModeMesh::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;
	const auto& selv = multi_view->selection[MultiViewType::MODEL_VERTEX];
	const auto& sele = multi_view->selection[MultiViewType::MODEL_EDGE];
	const auto& selp = multi_view->selection[MultiViewType::MODEL_POLYGON];

	if (presentation_mode == PresentationMode::Polygons or presentation_mode == PresentationMode::Surfaces or data->editing_mesh == data->phys_mesh.get())
		draw_polygons(params, win);

	if (data->editing_mesh == data->phys_mesh.get())
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_physical, material_physical, 0);

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_selection, material_selection, 0);

	VertexStagingBuffer vsb;
	if (multi_view->hover and multi_view->hover->type == MultiViewType::MODEL_POLYGON)
		data->editing_mesh->polygons[multi_view->hover->index].add_to_vertex_buffer(data->editing_mesh->vertices, vsb, 1);
	vsb.build(vertex_buffer_hover, 1);
	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_hover, material_hover, 0);

	if (presentation_mode == PresentationMode::Vertices or presentation_mode == PresentationMode::Edges or presentation_mode == PresentationMode::Polygons)
		draw_edges(params, win, sele);
}

base::optional<string> model_selection_description(DataModel* m, const Data::Selection& sel) {
	int nvert = 0, npoly = 0, nsphere = 0, ncyl = 0;
	if (sel.contains(MultiViewType::MODEL_VERTEX))
		nvert = sel[MultiViewType::MODEL_VERTEX].num;
	if (sel.contains(MultiViewType::MODEL_POLYGON))
		npoly = sel[MultiViewType::MODEL_POLYGON].num;
	if (sel.contains(MultiViewType::MODEL_BALL))
		nsphere = sel[MultiViewType::MODEL_BALL].num;
	if (sel.contains(MultiViewType::MODEL_CYLINDER))
		ncyl = sel[MultiViewType::MODEL_CYLINDER].num;
	if (nvert + npoly == 0)
		return base::None;
	Array<string> s;
	if (nvert > 0)
		s.add(format("%d vertices", nvert));
	if (npoly > 0)
		s.add(format("%d polygons", npoly));
	if (nsphere > 0)
		s.add(format("%d spheres", nsphere));
	if (ncyl > 0)
		s.add(format("%d cylinders", ncyl));
	return implode(s, ", ");
}

void ModeMesh::on_draw_post(Painter* p) {
	if (presentation_mode == PresentationMode::Vertices)
		drawing2d::draw_data_points(p, multi_view->active_window, data->editing_mesh->vertices, MultiViewType::MODEL_VERTEX, multi_view->hover, multi_view->selection[MultiViewType::MODEL_VERTEX]);

	if (auto s = model_selection_description(data, multi_view->selection))
		draw_info(p, "selected: " + *s);
}

void ModeMesh::on_update_selection() {
	update_selection_vb();
}

void ModeMesh::update_vb() {
	vertex_buffers.resize(data->materials.num);
	for (int i=0; i<vertex_buffers.num; i++) {
		if (!vertex_buffers[i])
			vertex_buffers[i] = new ygfx::VertexBuffer("3f,3f,2f");

		VertexStagingBuffer vsb;
		for (auto& p: data->mesh->polygons)
			if (p.material == i)
				p.add_to_vertex_buffer(data->mesh->vertices, vsb, 1);
		vsb.build(vertex_buffers[i], 1);
	}

	PolygonMesh m;
	for (const auto& b: data->editing_mesh->spheres)
		m.add(GeometrySphere(data->editing_mesh->vertices[b.index].pos, b.radius, 8));
	for (const auto& c: data->editing_mesh->cylinders)
		m.add(GeometryCylinder(data->editing_mesh->vertices[c.index[0]].pos, data->editing_mesh->vertices[c.index[1]].pos, c.radius, 1, 32));
	m.build(vertex_buffer_physical);

	// update material
	materials.resize(data->materials.num);
	for (int i=0; i<materials.num; i++) {
		if (!materials[i])
			materials[i] = create_material(session->ctx, White, 0.7f, 0.2f, Black);

		materials[i]->albedo = data->materials[i]->col.albedo;
		materials[i]->metal = data->materials[i]->col.metal;
		materials[i]->roughness = data->materials[i]->col.roughness;
		materials[i]->emission = data->materials[i]->col.emission;
		materials[i]->textures.resize(data->materials[i]->texture_levels.num);
		for (int k=0; k<data->materials[i]->texture_levels.num; k++)
			materials[i]->textures[k] = data->materials[i]->texture_levels[k]->texture;
	}
}


void ModeMesh::update_selection_vb() {
	VertexStagingBuffer vsb;
	const auto& selp = multi_view->selection[MultiViewType::MODEL_POLYGON];
	for (auto&& [i, p]: enumerate(data->editing_mesh->polygons))
		if (selp.contains(i))
			p.add_to_vertex_buffer(data->editing_mesh->vertices, vsb, 1);
	// TODO spheres/cylinders
	vsb.build(vertex_buffer_selection, 1);
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
	}
	if (presentation_mode == PresentationMode::Edges) {
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
	}
	if (presentation_mode == PresentationMode::Polygons) {
		vec3 tp;
		int index;
		if (data->editing_mesh->is_mouse_over(win, mat4::ID, m, tp, index, false))
			return Hover{MultiViewType::MODEL_POLYGON, index, tp};
	}
	return h;
}

void selection_edges_from_vertices(base::set<int>& sele, const base::set<int>& selv, const Array<Edge>& edges) {
	sele.clear();
	for (const auto& [i, e]: enumerate(edges)) {
		if (selv.contains(e.index[0]) and selv.contains(e.index[1]))
			sele.add(i);
	}
}

void selection_polygons_from_vertices(base::set<int>& selp, const base::set<int>& selv, const PolygonMesh& mesh) {
	selp.clear();
	for (const auto& [i, p]: enumerate(mesh.polygons)) {
		bool is_selected = true;
		for (const auto& s: p.side)
			is_selected &= selv.contains(s.vertex);
		if (is_selected)
			selp.add(i);
	}
}

void selection_vertices_from_edges(base::set<int>& selv, const base::set<int>& sele, const Array<Edge>& edges) {
	selv.clear();
	for (const auto& [i,e]: enumerate(edges))
		if (sele.contains(i)) {
			selv.add(e.index[0]);
			selv.add(e.index[1]);
		}
}

void selection_polygons_from_edges(base::set<int>& selp, const base::set<int>& sele, const PolygonMesh& mesh, const Array<Edge>& edges) {
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

void selection_vertices_from_polygons(base::set<int>& selv, const base::set<int>& selp, const PolygonMesh& mesh) {
	selv.clear();
	for (const auto& [i,p]: enumerate(mesh.polygons))
		if (selp.contains(i))
			for (const auto& s: p.side)
				selv.add(s.vertex);
}

void selection_edges_from_polygons(base::set<int>& sele, const base::set<int>& selp, const PolygonMesh& mesh, const Array<Edge>& edges) {
	sele.clear();
	for (const auto& [i,p]: enumerate(mesh.polygons))
		if (selp.contains(i))
			for (const auto& e: p.get_edges())
				sele.add(edges.find(e));
}

Data::Selection ModeMesh::select_in_rect(MultiViewWindow* win, const rect& r) {
	Data::Selection sel;
	sel.add({MultiViewType::MODEL_VERTEX, {}});
	sel.add({MultiViewType::MODEL_EDGE, {}});
	sel.add({MultiViewType::MODEL_POLYGON, {}});
	sel.add({MultiViewType::MODEL_BALL, {}});
	sel.add({MultiViewType::MODEL_CYLINDER, {}});

	auto selv = sel[MultiViewType::MODEL_VERTEX] = MultiView::select_points_in_rect(win, r, data->editing_mesh->vertices);
	if (presentation_mode == PresentationMode::Vertices) {
		sel[MultiViewType::MODEL_VERTEX] = selv;
	}
	if (presentation_mode == PresentationMode::Edges) {
		selection_edges_from_vertices(sel[MultiViewType::MODEL_EDGE], selv, edges_cached);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		selection_polygons_from_vertices(sel[MultiViewType::MODEL_POLYGON], selv, *data->editing_mesh);
	}
	return sel;
}

void ModeMesh::make_selection_consistent(Data::Selection &sel) const {
	if (presentation_mode == PresentationMode::Vertices) {
		selection_edges_from_vertices(sel[MultiViewType::MODEL_EDGE], sel[MultiViewType::MODEL_VERTEX], edges_cached);
		selection_polygons_from_vertices(sel[MultiViewType::MODEL_POLYGON], sel[MultiViewType::MODEL_VERTEX], *data->editing_mesh);
		/*for (auto& b: data->editing_mesh->spheres)
			b.is_selected = data->editing_mesh->vertices[b.index].is_selected;
		for (auto& c: data->editing_mesh->cylinders)
			c.is_selected = data->editing_mesh->vertices[c.index[0]].is_selected and data->editing_mesh->vertices[c.index[1]].is_selected;*/
	}
	if (presentation_mode == PresentationMode::Edges) {
		selection_vertices_from_edges(sel[MultiViewType::MODEL_VERTEX], sel[MultiViewType::MODEL_EDGE], edges_cached);
		selection_polygons_from_edges(sel[MultiViewType::MODEL_POLYGON], sel[MultiViewType::MODEL_EDGE], *data->editing_mesh, edges_cached);
	}
	if (presentation_mode == PresentationMode::Polygons) {
		selection_vertices_from_polygons(sel[MultiViewType::MODEL_VERTEX], sel[MultiViewType::MODEL_POLYGON], *data->editing_mesh);
		selection_edges_from_polygons(sel[MultiViewType::MODEL_EDGE], sel[MultiViewType::MODEL_POLYGON], *data->editing_mesh, edges_cached);
	}
}

base::optional<Box> ModeMesh::get_selection_box(const Data::Selection& sel) const {
	return MultiView::points_get_selection_box(data->editing_mesh->vertices, sel[MultiViewType::MODEL_VERTEX]);
}



void ModeMesh::on_command(const string& id) {
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
	if (id == "delete") {
		if (auto s = model_selection_description(data, multi_view->selection)) {
			data->delete_selection(multi_view->selection, presentation_mode == PresentationMode::Vertices);
			multi_view->clear_selection();
			session->set_message("deleted: " + *s);
		} else {
			session->set_message("nothing selected");
		}
	}
	_parent->on_command(id);
}

void ModeMesh::copy() {
	temp_mesh->clear();
	temp_mesh->add(data->editing_mesh->copy_geometry(multi_view->selection));
	if (temp_mesh->is_empty())
		session->set_message("nothing selected");
	else
		session->set_message("copied: " + *model_selection_description(data, multi_view->selection));
}

void ModeMesh::paste() {
	multi_view->clear_selection();
	if (temp_mesh->is_empty()) {
		session->set_message("nothing to paste");
	} else {
		doc->set_mode(new ModePaste(this));
	}
}


void ModeMesh::on_key_down(int key) {
	if (key == (xhui::KEY_CONTROL | xhui::KEY_B))
		doc->set_mode(new ModeBevelEdges(this));
	if (key == (xhui::KEY_CONTROL | xhui::KEY_X))
		doc->set_mode(new ModeExtrudePolygons(this));
}

void ModeMesh::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeMesh::optimize_view() {
	multi_view->view_port.suggest_for_box(data->bounding_box());
}




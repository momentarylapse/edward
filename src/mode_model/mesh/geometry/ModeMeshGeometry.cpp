//
// Created by Michael Ankele on 2025-02-20.
//

#include "ModeMeshGeometry.h"
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
#include "../ModeMesh.h"
#include "../../ModeModel.h"
#include "action/ActionModelMoveSelection.h"
#include "action/ActionModelAlignToGrid.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/polymesh/edit/InvertPolygons.h>
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


ModeMeshGeometry::ModeMeshGeometry(ModeMesh* parent) : SubMode(parent) {
	mode_mesh = parent;
	multi_view = parent->multi_view;
	data = parent->data;
	generic_data = data;
	menu_id = "menu-model-geometry";
	toolbar_id = parent->toolbar_id;

	temp_mesh = new ModelMesh();
}

ModeMeshGeometry::~ModeMeshGeometry() = default;


void ModeMeshGeometry::on_enter_rec() {
	doc->out_changed >> create_sink([this] {
		on_update_menu();
	});
}

void ModeMeshGeometry::on_connect_events_rec() {
}

void ModeMeshGeometry::on_leave_rec() {
	doc->out_changed.unsubscribe(this);
}

class MeshOpButtons : public xhui::Panel {
public:
	explicit MeshOpButtons(MultiView* multi_view) : xhui::Panel("mesh-op-buttons") {
		from_source(R"foodelim(
Dialog mesh-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button mouse-action 'T' 'tooltip=Left button action: move selection' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-vertex 'V' 'tooltip=Add Vertex' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-polygon 'P' 'tooltip=Add Polygon' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-cube 'Q' 'tooltip=Add cube' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-sphere 'S' 'tooltip=Add sphere' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-platonic 'P' 'tooltip=Add platonic body' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-from-lathe 'L' 'tooltip=Add rotational symmetric (lathe)' height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-cylinder 'C' 'tooltip=Add cylinder' height=50 width=50 padding=7 noexpandx ignorefocus
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

void ModeMeshGeometry::on_enter() {
	auto update = [this] {
		normals_dirty = true;
		mode_mesh->update_vb();
		mode_mesh->update_selection_vb();
		out_redraw();
	};

	auto win = session->win;

	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		return select_in_rect(win, r);
	};
	multi_view->f_make_selection_consistent = [this] (Selection& sel) {
		return make_selection_consistent(sel);
	};
	multi_view->f_get_selection_box = [this] (const Selection& sel) {
		return get_selection_box(sel);
	};
	multi_view->f_create_action = [this] {
		return new ActionModelMoveSelection(data->editing_mesh, multi_view->selection);
	};
	mode_mesh->set_edit_mesh(data->mesh.get());
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
			mode_mesh->update_vb();
			mode_mesh->update_selection_vb();
			out_redraw();
			normals_dirty = false;
		}
	});
}

void ModeMeshGeometry::on_connect_events() {

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
	doc->event("align_to_grid", [this] {
		data->execute(new ActionModelAlignToGrid(data->editing_mesh, multi_view->selection, [this] (const vec3& v) {
			return multi_view->snap_v(v);
		}));
		session->info(format("aligned to grid (%s)", multi_view->format_length(multi_view->active_window->get_grid_d())));
	});
	doc->event("bevel_edges", [this] {
		doc->set_mode(new ModeBevelEdges(this));
	});
	// extrude_triangles_independent
	doc->event("extrude_triangles", [this] {
		doc->set_mode(new ModeExtrudePolygons(this));
	});
	doc->event("invert_trias", [this] {
		auto ed = polymesh::invert_polygons(*data->mesh, multi_view->selection[MultiViewType::MODEL_POLYGON]);
		data->edit_mesh(ed);
	});


}


void ModeMeshGeometry::on_leave() {
	data->out_changed.unsubscribe(this);
	set_overlay_panel(nullptr);
}

void ModeMeshGeometry::on_update_topology() {
	edges_cached = data->editing_mesh->edges();
	update_edge_info();
}

void ModeMeshGeometry::update_edge_info() {
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

void ModeMeshGeometry::on_update_menu() {
	_parent->on_update_menu();
}


void ModeMeshGeometry::on_prepare_scene(const yrenderer::RenderParams& params) {
}

void ModeMeshGeometry::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low.srgb_to_linear()});
}

void ModeMeshGeometry::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->draw_mesh(params, win, true);
}

base::optional<string> model_selection_description(DataModel* m, const Selection& sel) {
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

void ModeMeshGeometry::on_draw_post(Painter* p) {
	if (mode_mesh->presentation_mode == ModeMesh::PresentationMode::Vertices)
		drawing2d::draw_data_points(p, multi_view->active_window, data->editing_mesh->vertices, MultiViewType::MODEL_VERTEX, multi_view->hover, multi_view->selection[MultiViewType::MODEL_VERTEX]);

	if (auto s = model_selection_description(data, multi_view->selection))
		draw_info(p, "selected: " + *s);
}

void ModeMeshGeometry::on_update_selection() {
	mode_mesh->update_selection_vb();
}

base::optional<Hover> ModeMeshGeometry::get_hover(MultiViewWindow* win, const vec2& m) const {
	return mode_mesh->get_hover(win, m);
}

Selection ModeMeshGeometry::select_in_rect(MultiViewWindow* win, const rect& r) {
	return mode_mesh->select_in_rect(win, r);
}

void ModeMeshGeometry::make_selection_consistent(Selection &sel) const {
	return mode_mesh->make_selection_consistent(sel);
}

base::optional<Box> ModeMeshGeometry::get_selection_box(const Selection& sel) const {
	return MultiView::points_get_selection_box(data->editing_mesh->vertices, sel[MultiViewType::MODEL_VERTEX]);
}



void ModeMeshGeometry::on_command(const string& id) {
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();
	if (id == "delete") {
		if (auto s = model_selection_description(data, multi_view->selection)) {
			data->delete_selection(multi_view->selection, mode_mesh->presentation_mode == ModeMesh::PresentationMode::Vertices);
			multi_view->clear_selection();
			session->info("deleted: " + *s);
		} else {
			session->warning("nothing selected");
		}
	}
	_parent->on_command(id);
}

void ModeMeshGeometry::copy() {
	temp_mesh->clear();
	temp_mesh->add(data->editing_mesh->copy_geometry(multi_view->selection));
	if (temp_mesh->is_empty())
		session->warning("nothing selected");
	else
		session->info("copied: " + *model_selection_description(data, multi_view->selection));
}

void ModeMeshGeometry::paste() {
	multi_view->clear_selection();
	if (temp_mesh->is_empty()) {
		session->warning("nothing to paste");
	} else {
		doc->set_mode(new ModePaste(this));
	}
}


void ModeMeshGeometry::on_key_down(int key) {
	if (key == (xhui::KEY_CONTROL | xhui::KEY_B))
		doc->set_mode(new ModeBevelEdges(this));
	if (key == (xhui::KEY_CONTROL | xhui::KEY_X))
		doc->set_mode(new ModeExtrudePolygons(this));
}

void ModeMeshGeometry::on_mouse_move(const vec2& m, const vec2& d) {
}

void ModeMeshGeometry::optimize_view() {
	multi_view->view_port.suggest_for_box(data->bounding_box());
}




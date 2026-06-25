//
// Created by michi on 6/15/26.
//

#include "ModeMeshNormals.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/xhui/xhui.h>
#include <lib/yrenderer/helper/LineHelper.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>


class MeshNormalsOpButtons : public xhui::Panel {
public:
	explicit MeshNormalsOpButtons(MultiView* multi_view) : xhui::Panel("mesh-op-buttons") {
		from_source(R"foodelim(
Dialog mesh-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button normal_this_hard 'F' 'tooltip=Make selected polygons flat' height=50 width=50 padding=7 noexpandx ignorefocus
		Button normal_this_smooth 'S' 'tooltip=Make selected polygons smooth / rounded' height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");
	}
};

ModeMeshNormals::ModeMeshNormals(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	data = mode_mesh->data;
	multi_view = mode_mesh->multi_view;
	generic_data = mode_mesh->generic_data;
	toolbar_id = "model-toolbar";
	menu_id = "menu-model-normals";
}

void ModeMeshNormals::on_enter() {
	mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(false);

	set_overlay_panel(new MeshNormalsOpButtons(multi_view));
}

void ModeMeshNormals::on_leave() {
	//set_side_panel(nullptr);
}

void ModeMeshNormals::on_connect_events() {
	// TODO action
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
}

void ModeMeshNormals::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_mesh->draw_mesh(params, win, true);

	const auto& selp = multi_view->selection[MultiViewType::MODEL_POLYGON];
	const auto& filter = mode_mesh->visibility_stack.get(MultiViewType::MODEL_POLYGON);

	const float l = win->multi_view->view_port.radius * 0.02f;

	Array<vec3> points, points_sel;
	for (const auto& [i, p]: enumerate(data->mesh->polygons))
		if (filter(i))
			for (const auto& s: p.side) {
				points.add(data->mesh->vertices[s.vertex].pos);
				points.add(data->mesh->vertices[s.vertex].pos + s.normal * l);
			}
	for (const auto& [i, p]: enumerate(data->mesh->polygons))
		if (selp.contains(i) and filter(i))
			for (const auto& s: p.side) {
				points_sel.add(data->mesh->vertices[s.vertex].pos);
				points_sel.add(data->mesh->vertices[s.vertex].pos + s.normal * l);
			}
	auto lh = session->line_helper;
	lh->begin_draw(params, win->rvd());
	lh->set_z_test(false);
	lh->set_color(White);
	lh->set_line_width(2.0f);
	lh->draw_lines(points, false);
	lh->set_color(Red);
	lh->set_line_width(3.0f);
	lh->draw_lines(points_sel, false);
}

void ModeMeshNormals::on_draw_post(Painter* p) {
	mode_mesh->on_draw_post(p);
}


void ModeMeshNormals::on_key_down(int key) {
}

void ModeMeshNormals::on_mouse_move(const vec2& m, const vec2& d) {
}


void ModeMeshNormals::on_left_button_down(const vec2& m) {
}


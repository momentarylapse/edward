//
// Created by Michael Ankele on 2025-06-29.
//

#include "ModeSkeleton.h"
#include "../mesh/ModeMesh.h"
#include "../ModeModel.h"
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include <view/MultiViewWindow.h>
#include <view/EdwardWindow.h>
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/xhui/Theme.h>

ModeSkeleton::ModeSkeleton(ModeModel* _parent) : SubMode(_parent) {
	parent = _parent;
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
}

ModeSkeleton::~ModeSkeleton() = default;

void ModeSkeleton::on_enter() {
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	//session->win->set_visible("overlay-button-grid-left", true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		return select_in_rect(win, r);
	};
	multi_view->f_make_selection_consistent = [this] (Data::Selection& sel) {
	};
	multi_view->f_get_selection_box = [this] (const Data::Selection& sel) {
		return get_selection_box(sel);
	};
	/*multi_view->f_create_action = [this] {
		return new ActionModelMoveSelection(data->editing_mesh, multi_view->selection);
	};*/
}

void ModeSkeleton::on_leave() {
}

void ModeSkeleton::on_enter_rec() {
}

void ModeSkeleton::on_leave_rec() {
}

void ModeSkeleton::optimize_view() {
}

void ModeSkeleton::on_command(const string& id) {
	parent->on_command(id);
}

void ModeSkeleton::on_key_down(int key) {
}

void ModeSkeleton::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeSkeleton::on_draw_post(Painter* p) {
	drawing2d::draw_data_points(p, multi_view->active_window, data->bone, MultiViewType::SKELETON_BONE, multi_view->hover, multi_view->selection[MultiViewType::SKELETON_BONE]);
}

void ModeSkeleton::on_prepare_scene(const RenderParams& params) {
}

void ModeSkeleton::on_draw_background(const RenderParams& params, RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low});
}

void ModeSkeleton::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	parent->mode_mesh->draw_polygons(params, win);
	parent->mode_mesh->draw_edges(params, win, {});
	auto dh = session->drawing_helper;

	Array<vec3> points;

	auto add_bone = [&points, win] (const vec3& parent, const vec3& child) {
		const vec3 m = parent * 0.8f + child * 0.2f;
		const vec3 dir = (child - parent).normalized();
		const vec3 t = vec3::cross(dir, win->direction()).normalized() * (child - parent).length() * 0.07f;
		points.add(parent);
		points.add(m + t);
		points.add(parent);
		points.add(m - t);
		points.add(child);
		points.add(m + t);
		points.add(child);
		points.add(m - t);
	};

	for (const auto& b: data->bone)
		if (b.parent >= 0)
			add_bone(data->bone[b.parent].pos, b.pos);
	dh->set_color(White);
	dh->set_line_width(DrawingHelper::LINE_MEDIUM);
	dh->set_z_test(false);
	dh->draw_lines(points, false);
	dh->set_z_test(true);
}

void ModeSkeleton::update_menu() {
}

base::optional<Hover> ModeSkeleton::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;
	//float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, b]: enumerate(data->bone)) {
		const auto pp = win->project(b.pos);
		if (pp.z <= 0 or pp.z >= 1)
			continue;
		if ((pp.xy() - m).length_fuzzy() > 10)
			continue;
		h = {MultiViewType::SKELETON_BONE, i, b.pos};
	}
	return h;
}

Data::Selection ModeSkeleton::select_in_rect(MultiViewWindow* win, const rect& r) {
	Data::Selection sel;
	sel.add({MultiViewType::SKELETON_BONE, MultiView::select_points_in_rect(win, r, data->bone)});
	return sel;
}

base::optional<Box> ModeSkeleton::get_selection_box(const Data::Selection& sel) const {
	return MultiView::points_get_selection_box(data->bone, sel[MultiViewType::SKELETON_BONE]);
}


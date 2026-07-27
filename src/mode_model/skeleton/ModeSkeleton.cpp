//
// Created by Michael Ankele on 2025-06-29.
//

#include "ModeSkeleton.h"
#include "ModeAddBone.h"
#include "action/ActionModelMoveBones.h"
#include "action/ActionModelDeleteBoneSelection.h"
#include "../mesh/ModeMesh.h"
#include "../ModeModel.h"
#include "../data/DataModel.h"
#include <view/DrawingHelper.h>
#include <view/multiview/MultiView.h>
#include <view/multiview/MultiViewWindow.h>
#include <view/multiview/ActionController.h>
#include <view/EdwardWindow.h>
#include <view/DocumentSession.h>
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/xhui/Theme.h>
#include <lib/yrenderer/helper/LineHelper.h>
#include <lib/xhui/Menu.h>

#include "lib/os/msg.h"
#include "storage/Storage.h"


ModeSkeleton::ModeSkeleton(ModeModel* _parent) : SubMode(_parent) {
	parent = _parent;
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
	toolbar_id = "model-toolbar";
	menu_id = "menu_skeleton";
}

ModeSkeleton::~ModeSkeleton() = default;


class SkeletonOpButtons : public xhui::Panel {
public:
	explicit SkeletonOpButtons(MultiView* multi_view) : xhui::Panel("skeleton-op-buttons") {
		from_source(R"foodelim(
Dialog skeleton-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button mouse-action '' tooltip='Left button action: move selection' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
		Button add-bone '+' tooltip='Add bone' height=50 width=50 padding=7 noexpandx ignorefocus
		Button attach-model 'M' tooltip='Attach model' height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");

		event("mouse-action", [this] {
			auto m = new xhui::Menu;
			m->add_item("mouse-action-move", "Move");
			m->add_item("mouse-action-rotate", "Rotate");
			m->add_item("mouse-action-scale", "Scale");
			m->open_popup(this);
		});
		event("mouse-action-move", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			ac->set_action_mode(MouseActionMode::MOVE);
			set_options("mouse-action", "image=rf-translate");
			set_tooltip("mouse-action", "Left button action: move selection");
		});
		event("mouse-action-rotate", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			ac->set_action_mode(MouseActionMode::ROTATE);
			set_options("mouse-action", "image=rf-rotate");
			set_tooltip("mouse-action", "Left button action: rotate selection");
		});
		event("mouse-action-scale", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			ac->set_action_mode(MouseActionMode::SCALE);
			set_options("mouse-action", "image=rf-scale");
			set_tooltip("mouse-action", "Left button action: scale selection");
		});
	}
};

void ModeSkeleton::on_enter() {
	parent->mode_mesh->set_presentation_mode(ModeMesh::PresentationMode::Polygons);
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		return select_in_rect(win, r);
	};
	multi_view->f_make_selection_consistent = [] (Selection& sel) {
	};
	multi_view->f_get_selection_box = [this] (const Selection& sel) {
		return get_selection_box(sel);
	};
	multi_view->f_create_action = [this] {
		return new ActionModelMoveBones(data, multi_view->selection);
	};
	set_overlay_panel(new SkeletonOpButtons(multi_view));
}

void ModeSkeleton::on_connect_events() {
	doc->event("add-bone", [this] {
		doc->set_mode(new ModeAddBone(this));
	});
	doc->event("attach-model", [this] {
		const auto sel = multi_view->selection;
		if (sel[MultiViewType::SKELETON_BONE].num == 0) {
			session->error("no bones selected");
			return;
		}
		session->storage->file_dialog(FD_MODEL, false, true).then([this, sel] (const ComplexPath& p) {
			for (int i: sel[MultiViewType::SKELETON_BONE])
				data->bone_attach_model(i, p.relative);
		});
	});
	doc->event("no-model", [this] {
		const auto sel = multi_view->selection;
		if (sel[MultiViewType::SKELETON_BONE].num == 0) {
			session->error("no bones selected");
			return;
		}
		for (int i: sel[MultiViewType::SKELETON_BONE])
			data->bone_attach_model(i, "");
	});
}

void ModeSkeleton::on_leave() {
	set_overlay_panel(nullptr);
}

void ModeSkeleton::on_enter_rec() {
}

void ModeSkeleton::on_leave_rec() {
}

void ModeSkeleton::optimize_view() {
}

base::optional<string> skeleton_selection_description(DataModel* m, const Selection& sel) {
	int nbones = 0;
	if (sel.contains(MultiViewType::SKELETON_BONE))
		nbones = sel[MultiViewType::SKELETON_BONE].num;
	if (nbones == 0)
		return base::None;
	return format("%d bones", nbones);
}

void ModeSkeleton::on_command(const string& id) {
	parent->on_command(id);
	if (id == "delete") {
		if (auto s = skeleton_selection_description(data, multi_view->selection)) {
			data->execute(new ActionModelDeleteBoneSelection(data, multi_view->selection[MultiViewType::SKELETON_BONE]));
			multi_view->clear_selection();
			session->info("deleted: " + *s);
		} else {
			session->warning("nothing selected");
		}
	}
}

void ModeSkeleton::on_key_down(int key) {
}

void ModeSkeleton::on_mouse_move(const vec2& m, const vec2& d) {
	out_redraw();
}

void ModeSkeleton::on_draw_post(Painter* p) {
	VisibilityStack dummy;
	drawing2d::draw_data_points(p, multi_view->active_window, data->bones, MultiViewType::SKELETON_BONE, multi_view->hover, multi_view->selection[MultiViewType::SKELETON_BONE], dummy.get(MultiViewType::SKELETON_BONE));

	if (auto s = skeleton_selection_description(data, multi_view->selection))
		draw_info(p, "selected: " + *s);
}

void ModeSkeleton::on_prepare_scene(const yrenderer::RenderParams& params) {
}

void ModeSkeleton::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low.srgb_to_linear()});
}

void ModeSkeleton::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	parent->mode_mesh->draw_mesh(params, win, false);

	auto lh = session->line_helper;
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;
	const auto& sel = multi_view->selection[MultiViewType::SKELETON_BONE];

	for (const auto& b: data->bones)
		if (b.model) {
			dh->draw_mesh(params, rvd, mat4::translation(b.pos), b.model->mesh[0]->sub[0].vertex_buffer, b.model->materials[0]);
		}

	auto draw_bone = [win, lh] (const vec3& parent, const vec3& child, bool selected) {
		Array<vec3> points;
		const vec3 m = parent * 0.8f + child * 0.2f;
		const vec3 dir = (child - parent).normalized();
		const vec3 t = vec3::cross(dir, win->direction()).normalized() * (child - parent).length() * 0.05f;
		points.add(parent);
		points.add(m + t);
		points.add(child);
		points.add(m - t);
		points.add(parent);
		if (selected) {
			lh->set_color(Red);
			lh->set_line_width(DrawingHelper::LINE_THICK);
		} else {
			lh->set_color(White);
			lh->set_line_width(DrawingHelper::LINE_MEDIUM);
		}
		lh->draw_lines(points, false);
	};

	lh->set_z_test(false);
	for (const auto& [i, b]: enumerate(data->bones))
		if (b.parent >= 0)
			draw_bone(data->bones[b.parent].pos, b.pos, sel.contains(i));
	lh->set_z_test(true);
}

void ModeSkeleton::on_update_menu() {
	parent->on_update_menu();
}

base::optional<Hover> ModeSkeleton::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;
	//float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, b]: enumerate(data->bones)) {
		const auto pp = win->project(b.pos);
		if (pp.z <= 0 or pp.z >= 1)
			continue;
		if ((pp.xy() - m).length_fuzzy() > 10)
			continue;
		h = {MultiViewType::SKELETON_BONE, i, b.pos};
	}
	return h;
}

Selection ModeSkeleton::select_in_rect(MultiViewWindow* win, const rect& r) {
	Selection sel;
	VisibilityStack dummy;
	sel.add({MultiViewType::SKELETON_BONE, MultiView::select_points_in_rect(win, r, data->bones, dummy.get(MultiViewType::SKELETON_BONE))});
	return sel;
}

base::optional<Box> ModeSkeleton::get_selection_box(const Selection& sel) const {
	return MultiView::points_get_selection_box(data->bones, sel[MultiViewType::SKELETON_BONE]);
}


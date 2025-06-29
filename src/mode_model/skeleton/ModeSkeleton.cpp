//
// Created by Michael Ankele on 2025-06-29.
//

#include "ModeSkeleton.h"
#include "../mesh/ModeMesh.h"
#include "../ModeModel.h"
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include <view/MultiViewWindow.h>
#include <Session.h>
#include <lib/xhui/Theme.h>

ModeSkeleton::ModeSkeleton(ModeModel* _parent) : SubMode(_parent) {
	parent = _parent;
	multi_view = parent->multi_view;
	data = parent->data.get();
	generic_data = data;
}

ModeSkeleton::~ModeSkeleton() = default;

void ModeSkeleton::on_enter() {
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

void ModeSkeleton::on_draw_post(Painter*) {
}

void ModeSkeleton::on_prepare_scene(const RenderParams& params) {
}

void ModeSkeleton::on_draw_background(const RenderParams& params, RenderViewData& rvd) {
	rvd.clear(params, {xhui::Theme::_default.background_low});
}

void ModeSkeleton::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	parent->mode_mesh->on_draw_win(params, win);
}

void ModeSkeleton::update_menu() {
}

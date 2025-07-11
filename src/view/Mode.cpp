//
// Created by Michael Ankele on 2025-01-20.
//

#include "Mode.h"

#include <Session.h>
#include <lib/image/Painter.h>
#include <lib/xhui/Theme.h>
#include <renderer/regions/RegionRenderer.h>
#include "DrawingHelper.h"
#include "EdwardWindow.h"

Mode::Mode(Session* _session) {
	session = _session;
	multi_view = nullptr;
}

void Mode::draw_info(Painter* p, const string& msg) {
	vec2 pos = p->area().p01() + vec2(30, -40);
	drawing2d::draw_boxed_str(p, pos, msg);
}

void Mode::set_side_panel(xhui::Panel* p) {
	if (side_panel)
		session->win->unembed(side_panel);
	side_panel = p;
	if (side_panel)
		session->win->embed("main-grid", 1, 0, side_panel);
}

Mode* Mode::get_root() {
	if (auto p = get_parent())
		return p->get_root();
	return this;
}


bool Mode::is_ancestor_of(Mode* m) {
	if (this == m)
		return true;
	if (auto p = m->get_parent())
		return is_ancestor_of(p);
	return false;
}


SubMode::SubMode(Mode* parent) : Mode(parent->session) {
	_parent = parent;
}

void SubMode::on_command(const string& id) {
	_parent->on_command(id);
}

void SubMode::on_draw_background(const RenderParams& params, RenderViewData& rvd) {
	_parent->on_draw_background(params, rvd);
}

void SubMode::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	_parent->on_draw_win(params, win);
}

void SubMode::on_draw_shadow(const RenderParams& params, RenderViewData& rvd) {
	_parent->on_draw_shadow(params, rvd);
}

void SubMode::on_prepare_scene(const RenderParams& params) {
	_parent->on_prepare_scene(params);
}






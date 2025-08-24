//
// Created by michi on 8/24/25.
//

#include "ModeScripting.h"
#include "ModeWorld.h"
#include "../view/MultiView.h"
#include "dialog/ScriptingPanel.h"
#include <lib/xhui/xhui.h>

ModeScripting::ModeScripting(ModeWorld* _mode_world) : SubMode(_mode_world) {
	mode_world = _mode_world;
	data = mode_world->data;
	multi_view = mode_world->multi_view;
	generic_data = data;
}

void ModeScripting::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return mode_world->get_hover(win, m);
	};
	multi_view->f_select = nullptr;
	multi_view->f_get_selection_box = nullptr;
	multi_view->f_create_action = nullptr;
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->entities}
	};

	set_side_panel(new ScriptingPanel(mode_world));
}

void ModeScripting::on_leave() {
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}

void ModeScripting::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_world->on_draw_win(params, win);
}

void ModeScripting::on_draw_post(Painter*) {

}

void ModeScripting::on_command(const string& id) {
	mode_world->on_command(id);
}



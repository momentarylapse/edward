//
// Created by michi on 9/9/25.
//

#include "ModeWorldProperties.h"
#include "ModeWorld.h"
#include "../view/MultiView.h"
#include "dialog/PropertiesDialog.h"
#include <lib/xhui/xhui.h>

ModeWorldProperties::ModeWorldProperties(ModeWorld* _mode_world) : SubMode(_mode_world) {
	mode_world = _mode_world;
	data = mode_world->data;
	multi_view = mode_world->multi_view;
	generic_data = data;
}

void ModeWorldProperties::on_enter() {
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

	set_side_panel(new PropertiesDialog(mode_world->data));
}

void ModeWorldProperties::on_leave() {
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}

void ModeWorldProperties::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_world->on_draw_win(params, win);
}

void ModeWorldProperties::on_draw_post(Painter*) {

}

void ModeWorldProperties::on_command(const string& id) {
	mode_world->on_command(id);
}
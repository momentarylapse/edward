//
// Created by Michael Ankele on 2025-04-21.
//

#include "ModeEditTerrain.h"
#include "ModeWorld.h"
#include "../view/MultiView.h"
#include "dialog/EditTerrainPanel.h"
#include <lib/xhui/xhui.h>

ModeEditTerrain::ModeEditTerrain(ModeWorld* _mode_world, int _index) : Mode(_mode_world->session) {
	mode_world = _mode_world;
	data = mode_world->data;
	multi_view = mode_world->multi_view;
	generic_data = data;
	index = _index;
}

void ModeEditTerrain::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return mode_world->get_hover(win, m);
	};
	multi_view->f_select = nullptr;
	multi_view->f_get_selection_box = nullptr;
	multi_view->f_create_action = nullptr;
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->entities}
	};

	set_side_panel(new EditTerrainPanel(this));
}

void ModeEditTerrain::on_leave() {
	//session->win->unembed(dialog);
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}

void ModeEditTerrain::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		session->set_mode(mode_world);
	}
}

void ModeEditTerrain::on_left_button_down(const vec2&) {

}

void ModeEditTerrain::on_left_button_up(const vec2&) {

}

void ModeEditTerrain::on_mouse_leave(const vec2&) {

}

void ModeEditTerrain::on_mouse_move(const vec2& m, const vec2& d) {

}

void ModeEditTerrain::on_prepare_scene(const RenderParams& params) {
	mode_world->on_prepare_scene(params);
}

void ModeEditTerrain::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_world->on_draw_win(params, win);
}

void ModeEditTerrain::on_draw_post(Painter*) {

}

WorldTerrain& ModeEditTerrain::terrain() const {
	return data->entities[index].terrain;
}

void ModeEditTerrain::on_command(const string& id) {
	mode_world->on_command(id);
}






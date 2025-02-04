//
// Created by Michael Ankele on 2025-02-04.
//

#include "ModeAddEntity.h"
#include "AddEntityPanel.h"
#include "ModeWorld.h"

ModeAddEntity::ModeAddEntity(ModeWorld* parent) :
	Mode(parent->session)
{
	mode_world = parent;
	multi_view = mode_world->multi_view;
	generic_data = mode_world->generic_data;
}

void ModeAddEntity::on_enter() {
	session->set_message("XXXX");
	mode_world->set_side_panel(new AddEntityPanel(mode_world));
}

Mode* ModeAddEntity::get_parent() {
	return mode_world;
}


void ModeAddEntity::on_prepare_scene(const RenderParams& params) {
	mode_world->on_prepare_scene(params);
}

void ModeAddEntity::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	mode_world->on_draw_win(params, win);
}




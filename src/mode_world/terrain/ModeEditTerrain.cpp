//
// Created by Michael Ankele on 2025-04-21.
//

#include "ModeEditTerrain.h"

#include <cmath>

#include "../ModeWorld.h"
#include <view/multiview/MultiView.h>
#include <view/DocumentSession.h>
#include "dialog/EditTerrainPanel.h"
#include <lib/xhui/xhui.h>
#include <ecs/Entity.h>
#include <y/world/Terrain.h>
#include <y/world/World.h>

#include "lib/os/msg.h"
#include "lib/yrenderer/helper/LineHelper.h"
#include "view/DrawingHelper.h"
#include "view/EdwardWindow.h"

ModeEditTerrain::ModeEditTerrain(ModeWorld* _mode_world, int _index) : SubMode(_mode_world) {
	mode_world = _mode_world;
	data = mode_world->data;
	multi_view = mode_world->multi_view;
	generic_data = data;
	index = _index;
	brush_radius = 500;
}

void ModeEditTerrain::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(true);
	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return mode_world->get_hover_terrains(win, m);
	};
	multi_view->f_select = nullptr;
	multi_view->f_get_selection_box = nullptr;
	multi_view->f_create_action = nullptr;
	multi_view->data_sets = {
		{MultiViewType::WORLD_ENTITY, &data->dummy_entities}
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
		doc->set_mode(mode_world);
	}
}

void ModeEditTerrain::on_left_button_down(const vec2&) {

	float depth = brush_radius / 4;
	if (session->win->is_key_pressed(xhui::KEY_SHIFT))
		depth = - depth;

	auto t = terrain();
	if (auto h = multi_view->hover) {
		if (h->index == index) {
			int N = (int)(brush_radius / t->pattern.x * 1.5f + 1);
			vec3 dp = h->tp - data->entity(index)->pos;
			float x = dp.x / t->pattern.x;
			float z = dp.z / t->pattern.z;

			int i0 = max((int)x - N, 0);
			int i1 = min((int)x + N, t->num_x);
			int j0 = max((int)z - N, 0);
			int j1 = min((int)z + N, t->num_z);
			for (int i = i0; i <= i1; i++)
				for (int j = j0; j <= j1; j++) {
					float dx = (float)i * t->pattern.x - dp.x;
					float dz = (float)j * t->pattern.z - dp.z;
					t->height[i*(t->num_z+1) + j] += depth * expf( -2 * (dx*dx + dz*dz) / (brush_radius * brush_radius));
				}
			t->update(i0, i1, j0, j1, -1);
			t->changed = true;
		}
	}
}

void ModeEditTerrain::on_left_button_up(const vec2&) {

}

void ModeEditTerrain::on_mouse_leave(const vec2&) {

}

void ModeEditTerrain::on_mouse_move(const vec2& m, const vec2& d) {

}

void ModeEditTerrain::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	mode_world->on_draw_win(params, win);

	auto t = terrain();
	if (auto h = multi_view->hover) {
		if (h->index == index) {
			vec3 n;
			t->gimme_height_n(data->entity(index), h->tp, n);
			session->line_helper->set_color(DrawingHelper::COLOR_X);
			session->line_helper->set_line_width(DrawingHelper::LINE_THIN);
			session->line_helper->set_z_test(false);
			session->line_helper->draw_circle(h->tp, n, brush_radius);
			session->line_helper->set_z_test(true);
		}
	}
}

void ModeEditTerrain::on_draw_post(Painter*) {

}

Terrain* ModeEditTerrain::terrain() const {
	return data->entity(index)->get_component<TerrainRef>()->terrain;
}

void ModeEditTerrain::on_command(const string& id) {
	mode_world->on_command(id);
}






//
// Created by Michael Ankele on 2025-04-21.
//

#pragma once


#include "../view/Mode.h"

class Terrain;
class DataWorld;
class ModeWorld;

class ModeEditTerrain : public SubMode {
public:
	explicit ModeEditTerrain(ModeWorld* mode_world, int index);

	void on_enter() override;
	void on_leave() override;

	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2&) override;
	void on_left_button_down(const vec2&) override;
	void on_left_button_up(const vec2&) override;
	void on_key_down(int key) override;
	void on_command(const string& id) override;

	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

	DataWorld* data;
	ModeWorld* mode_world;
	int index;

	Terrain* terrain() const;
};



//
// Created by Michael Ankele on 2025-04-21.
//

#ifndef MODEEDITTERRAIN_H
#define MODEEDITTERRAIN_H


#include "../view/Mode.h"

class DataWorld;
class ModeWorld;

class ModeEditTerrain : public Mode {
public:
	explicit ModeEditTerrain(ModeWorld* mode_world);

	void on_enter() override;
	void on_leave() override;

	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2&) override;
	void on_left_button_down(const vec2&) override;
	void on_left_button_up(const vec2&) override;
	void on_key_down(int key) override;

	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

	DataWorld* data;
	ModeWorld* mode_world;
};



#endif //MODEEDITTERRAIN_H

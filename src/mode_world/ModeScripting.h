//
// Created by michi on 8/24/25.
//

#pragma once

#include "../view/Mode.h"

class DataWorld;
class ModeWorld;

class ModeScripting : public SubMode {
public:
	explicit ModeScripting(ModeWorld* mode_world);

	void on_enter() override;
	void on_leave() override;

	void on_command(const string& id) override;

	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

	DataWorld* data;
	ModeWorld* mode_world;
};


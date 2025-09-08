//
// Created by michi on 9/9/25.
//

#pragma once
#include "../view/Mode.h"

class DataWorld;
class ModeWorld;

class ModeWorldProperties : public SubMode {
public:
	explicit ModeWorldProperties(ModeWorld* mode_world);

	void on_enter() override;
	void on_leave() override;

	void on_command(const string& id) override;

	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

	DataWorld* data;
	ModeWorld* mode_world;
};


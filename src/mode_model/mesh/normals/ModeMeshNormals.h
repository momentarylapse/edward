//
// Created by michi on 6/15/26.
//

#pragma once


#include <view/Mode.h>
#include "../../data/DataModel.h"

class ModeMesh;

class ModeMeshNormals : public SubMode {
public:
	explicit ModeMeshNormals(ModeMesh* parent);

	void on_enter() override;
	void on_leave() override;
	void on_connect_events() override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	ModeMesh* mode_mesh;
	DataModel* data;
};


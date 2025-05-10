//
// Created by Michael Ankele on 2025-02-21.
//

#ifndef MODEPASTE_H
#define MODEPASTE_H


#include "../../view/Mode.h"
#include "../data/DataModel.h"

class MultiViewWindow;
class ModeMesh;

class ModePaste : public SubMode {
public:
	explicit ModePaste(ModeMesh* parent);

	void on_enter() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_left_button_down(const vec2&) override;

	mat4 transformation;
	ModeMesh* mode_mesh;
	owned<VertexBuffer> vertex_buffer;
};


#endif //MODEPASTE_H

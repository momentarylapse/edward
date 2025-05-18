//
// Created by Michael Ankele on 2025-05-18.
//

#ifndef MODEEXTRUDEPOLYGONS_H
#define MODEEXTRUDEPOLYGONS_H


#include "../../view/Mode.h"

class MultiViewWindow;
class ModeMesh;

class ModeExtrudePolygons : public SubMode {
public:
	explicit ModeExtrudePolygons(ModeMesh* parent);

	void on_enter() override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	ModeMesh* mode_mesh;
};



#endif //MODEEXTRUDEPOLYGONS_H

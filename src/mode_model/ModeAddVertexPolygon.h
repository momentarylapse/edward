//
// Created by Michael Ankele on 2025-02-08.
//

#ifndef MODEADDVERTEXPOLYGON_H
#define MODEADDVERTEXPOLYGON_H


#include "../view/Mode.h"
#include "data/DataModel.h"

class MultiViewWindow;
class ModeModel;

class ModeAddVertexPolygon : public Mode {
public:
	explicit ModeAddVertexPolygon(ModeModel* parent);

	void on_enter() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;

	ModeModel* mode_model;
	Mode* get_parent() override;
};



#endif //MODEADDVERTEXPOLYGON_H

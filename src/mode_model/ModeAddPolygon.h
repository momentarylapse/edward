//
// Created by Michael Ankele on 2025-02-18.
//

#ifndef MODEADDPOLYGON_H
#define MODEADDPOLYGON_H


#include "../view/Mode.h"
#include "data/DataModel.h"

class MultiViewWindow;
class ModeModel;

class ModeAddPolygon : public Mode {
public:
	explicit ModeAddPolygon(ModeModel* parent);

	void on_enter() override;
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;
	void on_left_button_down(const vec2&) override;

	ModeModel* mode_model;
	Mode* get_parent() override;

	Array<int> vertices;
};




#endif //MODEADDPOLYGON_H

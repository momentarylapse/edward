#pragma once

#include "Control.h"

namespace xhui {

class Grid;
class Button;

class HeaderBar : public Control {
public:
	HeaderBar(Window *win, const string &id);

	void _draw(Painter *p) override;
	vec2 get_content_min_size() const override;
	void negotiate_area(const rect &available) override;

	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	Array<Control*> get_children(ChildFilter) const override;

	bool dragging = false;
	vec2 drag_m0;
	int window_pos_x0;
	int window_pos_y0;

	Grid *grid_right;
	Grid *grid_left;
	Button *button_close;
};

}

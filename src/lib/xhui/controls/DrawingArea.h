#pragma once

#include <functional>

#include "Control.h"

namespace xhui {



class DrawingArea : public Control {
public:
	explicit DrawingArea(const string &id);

	void _draw(Painter *p) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_middle_button_down(const vec2& m) override;
	void on_middle_button_up(const vec2& m) override;
	void on_right_button_down(const vec2& m) override;
	void on_right_button_up(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_mouse_wheel(const vec2& d) override;
	void on_key_down(int key) override;
	void on_key_up(int key) override;
	void on_key_char(int key) override;

	bool first_draw = true;

	void for_painter_do(Painter* p, std::function<void(Painter*)>);
};

}
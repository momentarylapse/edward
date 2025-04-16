//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef SLIDER_H
#define SLIDER_H

#include "Control.h"

namespace xhui {

class Slider : public Control {
public:
	explicit Slider(const string& id);
	void _draw(Painter* p) override;

	void set_float(float f) override;
	float get_float() override;

	float mouse_to_value(const vec2& m) const;

	vec2 get_content_min_size() const override;
	void negotiate_area(const rect& available) override;

	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_wheel(const vec2& d) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;

	void set_option(const string& key, const string& value) override;

	float button_radius;
	float value;
	float min, max, step;

	enum class State {
		DEFAULT,
		HOVER,
		PRESSED
	};
	State state;
};

} // xhui

#endif //SLIDER_H

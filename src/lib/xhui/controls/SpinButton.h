//
// Created by Michael Ankele on 2025-02-01.
//

#ifndef SPINBUTTON_H
#define SPINBUTTON_H

#include "Edit.h"

namespace xhui {

class SpinButton : public Edit {
public:
	SpinButton(const string& id, float value);

	float value;
	float _min, _max, step;
	int decimals;
	bool show_buttons = true;

	enum class Hover {
		Other,
		Plus,
		Minus
	};
	Hover hover;
	Hover pressed;
	Hover get_hover(const vec2& m) const;

	void set_float(float f) override;
	float get_float() override;
	void set_int(int i) override;
	int get_int() override;
	vec2 get_content_min_size() const override;
	void set_option(const string& key, const string& value) override;
	void _draw(Painter* p) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;
	void on_mouse_leave(const vec2& m) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_mouse_wheel(const vec2& d) override;

	void on_edit() override;
	void _update_text_from_value();
	void _update_value_from_text();
	bool text_needs_update = false;
};

} // xhui

#endif //SPINBUTTON_H

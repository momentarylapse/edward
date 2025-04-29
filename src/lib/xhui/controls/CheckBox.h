#pragma once

#include "Label.h"

namespace xhui {

class CheckBox : public Control {
public:
	CheckBox(const string &id, const string &title);

	vec2 get_content_min_size() const override;
	void negotiate_area(const rect& available) override;
	Array<Control*> get_children(ChildFilter f) const override;

	void check(bool checked) override;
	bool is_checked() override;

	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;

	void _draw(Painter *p) override;

	bool checked = false;
	enum class State {
		DEFAULT,
		HOVER,
		PRESSED
	} state;

	Label label;
};

}

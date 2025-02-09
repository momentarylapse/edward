#pragma once

#include "Label.h"

namespace xhui {

class Button : public Label {
public:
	Button(const string &id, const string &title);

	void get_content_min_size(int &w, int &h) const override;
	void enable(bool enabled) override;

	void on_mouse_enter(const vec2& m) override;
	void on_mouse_leave(const vec2& m) override;
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;

	void _draw(Painter *p) override;

	virtual void on_click();

	bool primary = false;
};

}

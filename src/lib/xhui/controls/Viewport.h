#pragma once

#include "Control.h"

namespace xhui {


class Viewport : public Control {
public:
	explicit Viewport(const string &id);

	void _draw(Painter *p) override;
	void on_mouse_wheel(const vec2& d) override;

	shared<Control> child;

	vec2 offset;
	vec2 content_size;

	void add_child(shared<Control> c, int x, int y) override;
	void remove_child(Control* c) override;

	vec2 get_greed_factor() const override;
	vec2 get_content_min_size() const override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void set_option(const string& key, const string& value) override;
};

}

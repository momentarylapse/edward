#pragma once

#include "Control.h"

namespace xhui {


class Overlay : public Control {
public:
	explicit Overlay(const string &id);

	void _draw(Painter *p) override;

	shared_array<Control> children;

	void add_child(shared<Control> c, int n);
	void add_child(shared<Control> c, int x, int y) override { add_child(c, x+y); }
	void remove_child(Control* c) override;

	vec2 get_content_min_size() const override;
	void negotiate_content_area(const rect &available) override;
	Array<const Node*> _get_children(ChildFilter) const override { return weak_as<const Node>(children); }
};

}
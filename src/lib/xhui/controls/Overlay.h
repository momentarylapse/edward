#pragma once

#include "Control.h"

namespace xhui {


class Overlay : public Control {
public:
	explicit Overlay(const string &id);

	void _draw(Painter *p) override;

	shared_array<Control> children;

	void add_child(shared<Control> c);
	void add_child(shared<Control> c, int, int) override { add_child(c); }
	void remove_child(Control* c) override;
	
	void get_greed_factor(float &x, float &y) const override;
	void get_content_min_size(int &w, int &h) const override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children(ChildFilter) const override { return weak(children); }
};

}
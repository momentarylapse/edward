#pragma once

#include "Control.h"

namespace xhui {


class Overlay : public Control {
public:
	explicit Overlay(const string &id);

	void _draw(Painter *p) override;

	Array<Control*> children;

	void add(Control *c);
	
	void get_greed_factor(float &x, float &y) override;
	void get_content_min_size(int &w, int &h) override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children() const override { return children; }
};

}
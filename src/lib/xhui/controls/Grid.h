#pragma once

#include "Control.h"

namespace xhui {


class Grid : public Control {
public:
	explicit Grid(const string &id);

	void _draw(Painter *p) override;

	struct Child {
		shared<Control> control;
		int x, y;
	};
	Array<Child> children;

	void add_child(shared<Control> c, int x, int y) override;
	void remove_child(Control* c) override;
	int nx = 0, ny = 0;
	float spacing;
	rect margin;
	bool card = false;
	bool vertical = false;

	void get_grid_min_sizes(Array<float> &w, Array<float> &h) const;
	void get_grid_greed_factors(Array<float> &x, Array<float> &y) const;
	
	vec2 get_greed_factor() const override;
	vec2 get_content_min_size() const override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void set_option(const string& key, const string& value) override;
};

}
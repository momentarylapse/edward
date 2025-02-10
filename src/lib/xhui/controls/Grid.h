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
	int spacing;
	int margin = 0;
	bool card = false;
	bool vertical = false;

	void get_grid_min_sizes(Array<int> &w, Array<int> &h) const;
	void get_grid_greed_factors(Array<float> &x, Array<float> &y) const;
	
	void get_greed_factor(float &x, float &y) const override;
	void get_content_min_size(int &w, int &h) const override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void set_option(const string& key, const string& value) override;
};

}
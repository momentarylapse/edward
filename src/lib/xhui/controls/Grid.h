#pragma once

#include "Control.h"

namespace xhui {


class Grid : public Control {
public:
	explicit Grid(const string &id);

	void _draw(Painter *p) override;

	struct Child {
		Control *control;
		int x, y;
	};
	Array<Child> children;

	void add(Control *c, int x, int y);
	int nx = 0, ny = 0;
	int spacing;

	void get_grid_min_sizes(Array<int> &w, Array<int> &h);
	void get_grid_greed_factors(Array<float> &x, Array<float> &y);
	
	void get_greed_factor(float &x, float &y) override;
	void get_content_min_size(int &w, int &h) override;
	void negotiate_area(const rect &available) override;
};

}
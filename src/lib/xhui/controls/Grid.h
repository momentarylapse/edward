#pragma once

#include "Control.h"
#include <lib/base/optional.h>
#include <lib/layout/Grid.h>

namespace xhui {


class Grid : public Control {
public:
	explicit Grid(const string &id);

	void _draw(Painter *p) override;

	void add_child(shared<Control> c, int x, int y) override;
	void remove_child(Control* c) override;
	bool card = false;
	float corner_radius = 0;
	base::optional<color> background;

	layout::Grid grid;
	
	vec2 get_greed_factor() const override;
	vec2 get_content_min_size() const override;
	void negotiate_area(const rect &available) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void set_option(const string& key, const string& value) override;
};

}

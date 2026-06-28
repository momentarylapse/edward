#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {



Grid::Grid(const string &_id) : Control(_id), grid(*this) {
	ignore_hover = true;
	grid.spacing = Theme::_default.spacing;
	padding = {0,0,0,0};

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Grid::add_child(shared<Control> c, int x, int y) {
	grid.add_child(c.get(), x, y);
	if (owner)
		c->_register(owner);
}

void Grid::remove_child(Control* c) {
	c->_unregister();
	grid.remove_child(c);
}


void Grid::_draw(Painter *p) {
	if (background) {
		p->set_roundness(corner_radius);
		p->set_color(*background);
		p->draw_rect(area);
		p->set_roundness(0);
	}

	for (auto &c: grid.children)
		if (c.node->visible)
			static_cast<Control*>(c.node.get())->_draw(p);
}

vec2 Grid::get_content_min_size() const {
	return grid.get_content_min_size();
}

vec2 Grid::get_greed_factor() const {
	return grid.get_greed_factor();
}

void Grid::negotiate_content_area(const rect &available) {
	grid.negotiate_content_area(available);
}

Array<const layout::Node*> Grid::_get_children(ChildFilter f) const {
	Array<const Node*> r;
	for (auto& c: grid.children)
		if (f == ChildFilter::All or c.node->visible)
			r.add(c.node.get());
	return r;
}

void Grid::set_option(const string& key, const string& value) {
	if (key == "class") {
		if (value == "card") {
			card = true;
			padding = {7,7,7,7};
			background = Theme::_default.background_raised();
			corner_radius = Theme::_default.button_radius;
		}
	} else if (key == "background") {
		background = color::parse(value);
	} else {
		grid.set_option(key, value);
		Control::set_option(key, value);
	}
	request_redraw();
}



}

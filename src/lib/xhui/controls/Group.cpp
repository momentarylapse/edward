//
// Created by Michael Ankele on 2025-02-02.
//

#include "Group.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"

namespace xhui {

Group::Group(const string& id, const string& _title) : Control(id) {
	title = _title;
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Group::set_string(const string& s) {
	title = s;
	request_redraw();
}

void Group::_draw(Painter* p) {
	p->set_color(Theme::_default.text_label);
	p->set_font(Theme::_default.font_name, Theme::_default.font_size * 1.3f, true, false);
	auto dim = font::get_text_dimensions(title);
	//p->draw_str({_area.x1, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
	p->draw_str({_area.x1, _area.y1}, title);

	if (child and child->visible)
		child->_draw(p);
}

void Group::add_child(shared<Control> c, int x, int y) {
	child = c;
	if (owner)
		c->_register(owner);
}

void Group::remove_child(Control* c) {
	c->_unregister();
	if (child == c)
		child = nullptr;
}



Array<Control*> Group::get_children(ChildFilter) const {
	if (child)
		return {child.get()};
	return {};
}

void Group::negotiate_area(const rect& available) {
	_area = available;
	if (child)
		child->negotiate_area({_area.p00() + vec2(0, 25), _area.p11()});
}

void Group::get_content_min_size(int& w, int& h) const {
	w = 0;
	h = 0;
	if (child)
		child->get_content_min_size(w, h);
	h += 25;
}

void Group::get_greed_factor(float& x, float& y) const {
	float sx, sy;
	if (child)
		child->get_greed_factor(sx, sy);
	x = y = 0;
	if (size_mode_x == SizeMode::Expand)
		x = 1;
	else if (size_mode_x == SizeMode::ForwardChild and child)
		x = sx;
	if (size_mode_y == SizeMode::Expand)
		y = 1;
	else if (size_mode_y == SizeMode::ForwardChild and child)
		y = sy;
}




} // xhui
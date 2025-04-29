//
// Created by Michael Ankele on 2025-02-02.
//

#include "Group.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"

namespace xhui {

static constexpr float SPACING = 4;

Group::Group(const string& id, const string& title) :
		Control(id),
		header(id + ":header", title)
{
	header.font_size = Theme::_default.font_size * 1.0f;
	header.bold = true;
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Group::set_string(const string& s) {
	header.set_string(s);
}

void Group::_draw(Painter* p) {
	header._draw(p);

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
		return {static_cast<Control*>(const_cast<Label*>(&header)), child.get()};
	return {static_cast<Control*>(const_cast<Label*>(&header))};
}

void Group::negotiate_area(const rect& available) {
	_area = available;
	float hh = header.get_content_min_size().y;
	header.negotiate_area({available.p00(), available.p10() + vec2(0, hh)});
	if (child)
		child->negotiate_area({_area.p00() + vec2(0, hh + SPACING), _area.p11()});
}

vec2 Group::get_content_min_size() const {
	vec2 s = header.get_effective_min_size();
	if (child) {
		vec2 cs = child->get_effective_min_size();
		s.x = max(s.x, cs.x);
		s.y += SPACING + cs.y;
	}
	return s;
}

vec2 Group::get_greed_factor() const {
	vec2 cf = {0, 0};
	if (child)
		cf = child->get_greed_factor();
	vec2 f = {0, 0};
	if (size_mode_x == SizeMode::Expand)
		f.x = 1;
	else if (size_mode_x == SizeMode::ForwardChild and child)
		f.x = cf.x;
	if (size_mode_y == SizeMode::Expand)
		f.y = 1;
	else if (size_mode_y == SizeMode::ForwardChild and child)
		f.y = cf.y;
	return f;
}




} // xhui
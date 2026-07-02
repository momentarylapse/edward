//
// Created by Michael Ankele on 2025-02-02.
//

#include "Group.h"
#include "../Painter.h"
#include "../Theme.h"
#include <lib/ygraphics/font.h>

namespace xhui {

static constexpr float SPACING = 4;

Group::Group(const string& id, const string& title) :
		Control(id),
		header(id + ":header", title)
{
	header.size_mode_x = SizeMode::Fill;
	header.font_size = Theme::_default.font_size * 1.0f;
	header.bold = true;
	ignore_hover = true;
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

Array<const layout::Node*> Group::_get_children(ChildFilter f) const {
	if (child)
		if (f == ChildFilter::All or child->visible)
			return {&header, child.get()};
	return {&header};
}

void Group::negotiate_content_area(const rect& available) {
	float hh = header.effective_min_size().y;
	header.negotiate_outer_area({available.p00(), available.p10() + vec2(0, hh)});
	if (child and child->visible)
		child->negotiate_outer_area({available.p00() + vec2(0, hh + SPACING), available.p11()});
}

vec2 Group::get_content_min_size() const {
	vec2 s = header.effective_min_size();
	if (child and child->visible) {
		vec2 cs = child->effective_min_size();
		s.x = max(s.x, cs.x);
		s.y += SPACING + cs.y;
	}
	return s;
}




} // xhui

#include "Overlay.h"

#include <lib/base/algo.h>

#include "../Panel.h"
#include "../../os/msg.h"

namespace xhui {




Overlay::Overlay(const string &_id) : Control(_id) {
	ignore_hover = true;

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Overlay::add_child(shared<Control> c) {
	children.add(c);
	if (owner)
		c->_register(owner);
}

void Overlay::remove_child(Control* c) {
	c->_unregister();
	base::remove_if(children, [c] (const auto& child) {
		return child.get() == c;
	});
}


void Overlay::_draw(Painter *p) {
	for (auto c: children)
		if (c->visible)
			c->_draw(p);
}

vec2 Overlay::get_content_min_size() const {
	vec2 s = {0, 0};
	for (auto c: children)
		s = vec2::max(s, c->get_effective_min_size());
	return s;
}

vec2 Overlay::get_greed_factor() const {
	// assume SizeMode::Forward...
	for (auto c: children)
		// first child
		return c->get_greed_factor();
	return {0,0};
}

void Overlay::negotiate_area(const rect &available) {
	_area = available;
	for (auto c: children)
		c->negotiate_area(available);
}

}

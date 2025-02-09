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

void Overlay::get_content_min_size(int &_w, int &_h) const {
	_w = 0;
	_h = 0;
	for (auto c: children) {
		int w, h;
		c->get_content_min_size(w, h);
		_w = max(_w, w);
		_h = max(_h, h);
	}
}

void Overlay::get_greed_factor(float &_x, float &_y) const {
	// SizeMode::Forward...
	_x = 0;
	_y = 0;
	for (auto c: children) {
		float x, y;
		c->get_greed_factor(x, y);
		_x = max(_x, x);
		_y = max(_y, y);
	}
}

void Overlay::negotiate_area(const rect &available) {
	_area = available;
	for (auto c: children)
		c->negotiate_area(available);
}

}

#include "Overlay.h"
#include "../Panel.h"
#include "../../os/msg.h"

namespace xhui {




Overlay::Overlay(const string &_id) : Control(_id) {
	ignore_hover = true;

	expand_x = true;
	expand_y = true;
}

void Overlay::add(Control *c) {
	children.add(c);
	c->_register(owner);
}

void Overlay::_draw(Painter *p) {
	for (auto c: children)
		c->_draw(p);
}

void Overlay::get_content_min_size(int &_w, int &_h) {
	_w = 0;
	_h = 0;
	for (auto c: children) {
		int w, h;
		c->get_content_min_size(w, h);
		_w = max(_w, w);
		_h = max(_h, h);
	}
}

void Overlay::get_greed_factor(float &_x, float &_y) {
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

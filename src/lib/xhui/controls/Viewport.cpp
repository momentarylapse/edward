#include "Viewport.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {


Viewport::Viewport(const string &_id) : Control(_id) {
	//ignore_hover = true;

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Viewport::add_child(shared<Control> c, int x, int y) {
	child = c;
	if (owner)
		c->_register(owner);
}

void Viewport::remove_child(Control* c) {
	if (child.get() == c) {
		c->_unregister();
		child = nullptr;
	}
}

void Viewport::on_mouse_wheel(const vec2& d) {
	if (child) {
		int w, h;
		child->get_effective_min_size(w, h);
		content_size = {(float)w, (float)h};
		offset -= d * 3;
		offset.x = clamp(offset.x, 0.0f, content_size.x - _area.width());
		offset.y = clamp(offset.y, 0.0f, content_size.y - _area.height());
		child->negotiate_area({_area.p00() - offset, _area.p00() + content_size - offset});
	}
	request_redraw();
}


void Viewport::_draw(Painter *p) {
	auto clip0 = p->clip();
	if (child and child->visible) {
		p->set_clip(_area);
		child->_draw(p);
		p->set_clip(clip0);
	}
}

void Viewport::get_content_min_size(int &_w, int &_h) const {
	_w = min_width_user;
	_h = min_height_user;
	if (child) {
		int w, h;
		child->get_content_min_size(w, h);
		if (size_mode_x == SizeMode::ForwardChild)
			_w = w;
		if (size_mode_y == SizeMode::ForwardChild)
			_h = h;
	}
}

void Viewport::get_greed_factor(float &_x, float &_y) const {
	_x = 0;
	_y = 0;
	if (child) {
		float cx, cy;
		child->get_greed_factor(cx, cy);
		if (size_mode_x == SizeMode::Expand)
			_x = 1;
		else if (size_mode_x == SizeMode::ForwardChild)
			_x = cx;
		if (size_mode_y == SizeMode::Expand)
			_y = 1;
		else if (size_mode_y == SizeMode::ForwardChild)
			_y = cy;
	}
}

void Viewport::negotiate_area(const rect &available) {
	_area = available;

	if (child) {
		rect content_area = {_area.p00() - offset, _area.p00() + content_size - offset};
		if (size_mode_x == SizeMode::ForwardChild) {
			content_area.x1 = _area.x1;
			content_area.x2 = _area.x2;
		}
		if (size_mode_y == SizeMode::ForwardChild) {
			content_area.y1 = _area.y1;
			content_area.y2 = _area.y2;
		}
		child->negotiate_area(content_area);
	}
}

Array<Control*> Viewport::get_children(ChildFilter) const {
	if (child)
		return {child.get()};
	return {};
}

void Viewport::set_option(const string& key, const string& value) {
	if (key == "...") {
	} else {
		Control::set_option(key, value);
	}
	request_redraw();
}



}

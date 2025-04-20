#include "Viewport.h"

#include <lib/os/msg.h>

#include "../Painter.h"
#include "../Theme.h"

namespace xhui {


Viewport::Viewport(const string &_id) : Control(_id) {
	//ignore_hover = true;

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
	offset = {0, 0};
	content_size = {0, 0};
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
		content_size = child->get_effective_min_size();
#ifdef OS_LINUX
		constexpr float speed = 30;
#else
		constexpr float speed = 3;
#endif
		offset -= d * speed;
		offset.x = clamp(offset.x, 0.0f, content_size.x - _area.width());
		offset.y = clamp(offset.y, 0.0f, content_size.y - _area.height());
		child->negotiate_area({_area.p00() - offset, _area.p00() + content_size - offset});
	}
	request_redraw();
}


void Viewport::_draw(Painter *p) {
	if (child and child->visible) {
		// hack :P
		content_size = child->get_effective_min_size();

		auto clip0 = p->clip();
		p->set_clip(_area);
		child->_draw(p);
		p->set_clip(clip0);
	}
}

vec2 Viewport::get_content_min_size() const {
	vec2 s = {min_width_user, min_height_user};
	if (child) {
		vec2 cs = child->get_effective_min_size();
		if (size_mode_x == SizeMode::ForwardChild)
			s.x = cs.x;
		if (size_mode_y == SizeMode::ForwardChild)
			s.y = cs.y;
	}
	return s;
}

vec2 Viewport::get_greed_factor() const {
	vec2 f = {0, 0};
	if (child) {
		vec2 cf = child->get_greed_factor();
		if (size_mode_x == SizeMode::Expand)
			f.x = 1;
		else if (size_mode_x == SizeMode::ForwardChild)
			f.x = cf.x;
		if (size_mode_y == SizeMode::Expand)
			f.y = 1;
		else if (size_mode_y == SizeMode::ForwardChild)
			f.y = cf.y;
	}
	return f;
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

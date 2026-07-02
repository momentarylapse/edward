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
		content_size = child->effective_min_size();
#ifdef OS_LINUX
		constexpr float speed = 30;
#else
		constexpr float speed = 3;
#endif
		offset -= d * speed;
		offset.x = clamp(offset.x, 0.0f, content_size.x - area.width());
		offset.y = clamp(offset.y, 0.0f, content_size.y - area.height());
		child->negotiate_outer_area({area.p00() - offset, area.p00() + content_size - offset});
	}
	request_redraw();
}


void Viewport::_draw(Painter *p) {
	if (child and child->visible) {
		// hack :P
		content_size = child->effective_min_size();

		auto clip0 = p->clip();
		p->set_clip(area and clip0);
		child->_draw(p);
		p->set_clip(clip0);
	}
}

vec2 Viewport::get_content_min_size() const {
	vec2 s = {min_width_user, min_height_user};
	if (child) {
		vec2 cs = child->effective_min_size();
		if (size_mode_x == SizeMode::ForwardChild)
			s.x = cs.x;
		if (size_mode_y == SizeMode::ForwardChild)
			s.y = cs.y;
	}
	return s;
}

void Viewport::negotiate_content_area(const rect &available) {
	if (child) {
		rect content_area = {available.p00() - offset, available.p00() + content_size - offset};
		if (size_mode_x == SizeMode::ForwardChild) {
			content_area.x1 = available.x1;
			content_area.x2 = available.x2;
		}
		if (size_mode_y == SizeMode::ForwardChild) {
			content_area.y1 = available.y1;
			content_area.y2 = available.y2;
		}
		child->negotiate_outer_area(content_area);
	}
}

Array<const layout::Node*> Viewport::_get_children(ChildFilter) const {
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

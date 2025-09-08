//
// Created by Michael Ankele on 2025-04-16.
//

#include "Expander.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

static constexpr float SPACING = 4;

Expander::Expander(const string& id, const string& title) :
		Control(id),
		header(id + ":header", title)
{
	header.font_size = Theme::_default.font_size * 1.0f;
	header.bold = true;
	show_header = (title != "");
	ignore_hover = true; // TODO interactive expand-button/header
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
	state = State::Undecided;
}

void Expander::set_string(const string& s) {
	header.set_string(s);
	show_header = (s != "");
}

void Expander::expand(bool expanded) {
	if (expanded and (state == State::Expanded or state == State::Expanding))
		return;
	if (!expanded and (state == State::Compact or state == State::Shrinking))
		return;
	if (state == State::Undecided) {
		state = expanded ? State::Expanded : State::Compact;
		animator.t = expanded ? 1 : 0;
		return;
	}

	animator.duration = 0.200f;
	if (expanded) {
		state = State::Expanding;
		animator.t0 = 0;
		animator.t1 = 1;
		animator.on_end = [this] {
			state = State::Expanded;
		};
	} else {
		state = State::Shrinking;
		animator.t0 = 1;
		animator.t1 = 0;
		animator.on_end = [this] {
			state = State::Compact;
		};
	}
	animator.start();
}

void Expander::_draw(Painter* p) {
	if (show_header)
		header._draw(p);

	if (child and child->visible and state != State::Compact) {
		const auto c0 = p->clip();
		if (state != State::Expanded)
			p->set_clip(_area and c0);
		child->_draw(p);
		if (state != State::Expanded)
			p->set_clip(c0);
	}
}

void Expander::add_child(shared<Control> c, int x, int y) {
	child = c;
	if (owner)
		c->_register(owner);
}

void Expander::remove_child(Control* c) {
	c->_unregister();
	if (child == c)
		child = nullptr;
}

Array<Control*> Expander::get_children(ChildFilter f) const {
	if (child and (state == State::Expanded or f == ChildFilter::All))
		return {static_cast<Control*>(const_cast<Label*>(&header)), child.get()};
	return {static_cast<Control*>(const_cast<Label*>(&header))};
}

void Expander::negotiate_area(const rect& available) {
	if (state == State::Undecided)
		state = State::Compact;
	_area = available;
	float hh = 0;
	if (show_header)
		hh = header.get_content_min_size().y;
	header.negotiate_area({available.p00(), available.p10() + vec2(0, hh)});
	if (child)
		child->negotiate_area({_area.p00() + vec2(0, hh + SPACING), _area.p11()});
}

vec2 Expander::get_content_min_size() const {
	vec2 s = {0,0};
	if (show_header)
		s += header.get_effective_min_size();
	if (child) {
		vec2 cs = child->get_effective_min_size();
		vec2 max_size = {max(s.x, cs.x), s.y + SPACING + cs.y};
		if (state == State::Expanded) {
			s = max_size;
		} else if (state == State::Expanding or state == State::Shrinking) {
			s = s + animator.t * (max_size - s);
		}
	}
	return s;
}

vec2 Expander::get_greed_factor() const {
	vec2 cf = {0, 0};
	if (child and state == State::Expanded)
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

void Expander::set_option(const string &key, const string &value) {
	if (key == "expanded") {
		expand(value == "" or value._bool());
	} else {
		Control::set_option(key, value);
	}
}



} // xhui
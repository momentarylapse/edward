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
	size_mode_x = SizeMode::Fill;//ForwardChild;
	size_mode_y = SizeMode::Shrink;//ForwardChild; // not sure... (-_-)'
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
		return;
	}

	animator.duration = 0.220f;
	animator.mode = Animator::Mode::EASE_OUT;
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
			p->set_clip(area and c0);
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

Array<const layout::Node*> Expander::_get_children(ChildFilter f) const {
	if (child and (state == State::Expanded or f == ChildFilter::All))
		return {&header, child.get()};
	return {&header};
}

void Expander::negotiate_content_area(const rect& available) {
	if (state == State::Undecided)
		state = State::Compact;
	float hh = 0;
	if (show_header)
		hh = header.effective_min_size().y;
	header.negotiate_outer_area({available.p00(), available.p10() + vec2(0, hh)});
	if (child)
		child->negotiate_outer_area({available.p00() + vec2(0, hh + SPACING), available.p11()});
}

vec2 Expander::get_content_min_size() const {
	vec2 s = {0,0};
	if (show_header)
		s += header.effective_min_size();
	if (child) {
		vec2 cs = child->effective_min_size();
		vec2 expanded_min_size = {max(s.x, cs.x), s.y + SPACING + cs.y};
		if (state == State::Expanded) {
			s = expanded_min_size;
		} else if (state == State::Expanding or state == State::Shrinking) {
			s.x = expanded_min_size.x;
			s.y = s.y + animator.t * (expanded_min_size.y - s.y);
		}
	}
	return s;
}

void Expander::set_option(const string &key, const string &value) {
	if (key == "expanded") {
		expand(value == "" or value._bool());
	} else if (key == "markup") {
		header.set_option(key, value);
	} else {
		Control::set_option(key, value);
	}
}



} // xhui
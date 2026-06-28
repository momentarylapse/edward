#include "Button.h"
#include <lib/os/msg.h>
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) :
		Control(_id, ControlType::Button),
		label(_id + ":label", t)
{
	state = State::DEFAULT;
	can_grab_focus = true;
	size_mode_x = SizeMode::Fill;
	size_mode_y = SizeMode::Fill;
	label.align = Label::Align::Center;
	label.padding = {0,0,0,0};
	label.bold = true;
	padding.x1 = padding.x2 = Theme::_default.button_margin_x;
	padding.y1 = padding.y2 = Theme::_default.button_margin_y;
}

void Button::on_click() {
	emit_event(event_id::Click, true);
}

void Button::enable(bool enabled) {
	Control::enable(enabled);
	label.enable(enabled);
}

void Button::on_left_button_down(const vec2&) {
	if (enabled)
		state = State::PRESSED;
	request_redraw();
	emit_event(event_id::LeftButtonDown, false);
}

void Button::on_left_button_up(const vec2& m) {
	if (enabled)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::LeftButtonUp, false);
	if (enabled and area.inside(m))
		on_click();
}

void Button::on_mouse_enter(const vec2&) {
	if (enabled)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::MouseEnter, false);
}

void Button::on_mouse_leave(const vec2&) {
	if (enabled)
		state = State::DEFAULT;
	request_redraw();
	emit_event(event_id::MouseLeave, false);
}

vec2 Button::get_content_min_size() const {
	return label.get_effective_min_size();
}

void Button::negotiate_content_area(const rect& available) {
	label.negotiate_outer_area(available);
}

Array<const layout::Node*> Button::_get_children(ChildFilter f) const {
	return {&label};
}


void Button::set_string(const string& s) {
	label.set_string(s);
}

string Button::get_string() {
	return label.get_string();
}

void Button::_draw(Painter *p) {
	color bg = Theme::_default.background_button;
	if (danger) {
		if (state == State::HOVER) {
			bg = Theme::_default.background_button_danger_hover;
		} else if (state == State::PRESSED) {
			bg = Theme::_default.background_button_danger_active;
		} else {
			bg = Theme::_default.background_button_danger;
		}
	} else if (primary) {
		if (state == State::HOVER) {
			bg = Theme::_default.background_button_primary_hover;
		} else if (state == State::PRESSED) {
			bg = Theme::_default.background_button_primary_active;
		} else {
			bg = Theme::_default.background_button_primary;
		}
	} else {
		if (state == State::HOVER) {
			bg = Theme::_default.background_hover;
		} else if (state == State::PRESSED) {
			bg = Theme::_default.background_active;
		}
	}
	if (primary or state != State::DEFAULT or !flat) {
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(area);
		p->set_roundness(0);
	}

	label._draw(p);
}

void Button::set_option(const string& key, const string& value) {
	if (key == "image" or key == "align") {
		label.set_option(key, value);
	} else if (key == "primary") {
		primary = true;
		request_redraw();
	} else if (key == "danger") {
		danger = true;
		request_redraw();
	} else if (key == "default") {
		_default = true;
		primary = true;
		request_redraw();
	} else if (key == "flat") {
		flat = true;
		request_redraw();
	} else if (key == "url") {
		flat = true;
		//padding = {0, 0, 0, 0};
		//label.margin.y1 = label.margin.y2 = Theme::_default.label_margin_y;
		label.set_option("bold", "false");
		label.set_option("url", "true");
		request_redraw();
	} else if (key == "small" or key == "small" or key == "bold" or key == "ellipsis") {
		label.set_option(key, value);
	} else {
		Control::set_option(key, value);
	}
}


}

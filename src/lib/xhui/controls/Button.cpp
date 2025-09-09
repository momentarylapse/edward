#include "Button.h"
#include <lib/os/msg.h>
#include "../Painter.h"
#include <lib/ygraphics/font.h>
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) :
		Control(_id, ControlType::Button),
		label(_id + ":label", t)
{
	state = State::DEFAULT;
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
	label.align = Label::Align::Center;
	label.margin = {0,0,0,0};
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

void Button::on_left_button_up(const vec2&) {
	if (enabled)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::LeftButtonUp, false);
	if (enabled)
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
	return label.get_content_min_size() + padding.p00() + padding.p11();
}

void Button::negotiate_area(const rect& available) {
	Control::negotiate_area(available);
	label.negotiate_area({available.p00() + padding.p00(), available.p11() - padding.p11()});
}

Array<Control*> Button::get_children(ChildFilter f) const {
	return {static_cast<Control*>(const_cast<Label*>(&label))};
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
		p->draw_rect(_area);
		p->set_roundness(0);
	}

	label._draw(p);
}

void Button::set_option(const string& key, const string& value) {
	if (key == "image" or key == "align") {
		label.set_option(key, value);
	} else if (key == "padding") {
		float f = value._float();
		padding = {f, f, f, f};
		request_redraw();
	} else if (key == "paddingx") {
		float f = value._float();
		padding.x1 = padding.x2 = f;
		request_redraw();
	} else if (key == "paddingy") {
		float f = value._float();
		padding.y1 = padding.y2 = f;
		request_redraw();
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
	} else if (key == "small" or key == "small" or key == "bold") {
		label.set_option(key, value);
	} else {
		Control::set_option(key, value);
	}
}


}

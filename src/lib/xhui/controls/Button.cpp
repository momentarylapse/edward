#include "Button.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) :
		Control(_id),
		label(_id + ":label", t)
{
	state = State::DEFAULT;
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
	label.align = Label::Align::Center;
	label.margin_x = Theme::_default.button_margin_x;
	label.margin_y = Theme::_default.button_margin_y;
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

void Button::get_content_min_size(int &w, int &h) const {
	label.get_content_min_size(w, h);
}

void Button::negotiate_area(const rect& available) {
	Control::negotiate_area(available);
	label.negotiate_area(available);
}


void Button::_draw(Painter *p) {
	color bg = Theme::_default.background_button;
	if (primary) {
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
	p->set_color(bg);
	p->set_roundness(Theme::_default.button_radius);
	p->draw_rect(_area);
	p->set_roundness(0);

	label._draw(p);
}

void Button::set_option(const string& key, const string& value) {
	if (key == "image" or key == "align") {
		label.set_option(key, value);
	} else {
		Control::set_option(key, value);
	}
}


}

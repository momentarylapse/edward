#include "Button.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) : Label(_id, t) {
	state = State::DEFAULT;
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
	align = Align::Center;
	margin_x = Theme::_default.label_margin_x;
}

void Button::on_click() {
	emit_event(event_id::Click, true);
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
	if (text_w < 0) {
		font::set_font(Theme::_default.font_name, Theme::_default.font_size * ui_scale);
		auto dim = font::get_text_dimensions(title);
		text_w = int(dim.bounding_width / ui_scale);
		text_h = int(dim.inner_height() / ui_scale);
	}
	w = text_w + (int)Theme::_default.button_margin_x * 2;
	h = text_h + (int)Theme::_default.button_margin_y * 2;
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

	Label::_draw(p);
}

}

#include "Button.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) : Label(_id, t) {
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
}

void Button::enable(bool enabled) {
	if (enabled)
		state = State::DEFAULT;
	else
		state = State::DISABLED;
	request_redraw();
}

void Button::on_click() {
	emit_event(event_id::Click, true);
}


void Button::on_left_button_down(const vec2&) {
	if (state != State::DISABLED)
		state = State::PRESSED;
	request_redraw();
	emit_event(event_id::LeftButtonDown, false);
}

void Button::on_left_button_up(const vec2&) {
	if (state != State::DISABLED)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::LeftButtonUp, false);
	on_click();
}
void Button::on_mouse_enter(const vec2&) {
	if (state != State::DISABLED)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::MouseEnter, false);
}
void Button::on_mouse_leave(const vec2&) {
	if (state != State::DISABLED)
		state = State::DEFAULT;
	request_redraw();
	emit_event(event_id::MouseLeave, false);
}

void Button::get_content_min_size(int &w, int &h) {
	if (text_w < 0) {
		font::set_font(Theme::_default.font_name, Theme::_default.font_size);
		auto dim = font::get_text_dimensions(title);
		text_w = int(dim.bounding_width);
		text_h = int(dim.inner_height());
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

	p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
	auto dim = font::get_text_dimensions(title);

	//p->set_color(Red);
	//p->draw_rect({_area.center().x - dim.bounding_width/2, _area.center().x + dim.bounding_width/2 , _area.center().y - dim.bounding_height/2, _area.center().y + dim.bounding_height/2});
	p->set_color(Theme::_default.text);
	if (state == State::DISABLED)
		p->set_color(Theme::_default.text_disabled);
	p->draw_str({_area.center().x - dim.bounding_width / ui_scale / 2, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
}

}

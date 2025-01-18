#include "Button.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

Button::Button(const string &_id, const string &t) : Label(_id, t) {
	can_grab_focus = true;
	expand_x = true;
	expand_y = false;
}

void Button::on_left_button_down(const vec2&) {
	state = State::PRESSED;
	request_redraw();
}
void Button::on_left_button_up(const vec2&) {
	state = State::HOVER;
	request_redraw();

	if (owner)
		owner->handle_event(id, "hui:click");
}
void Button::on_mouse_enter(const vec2&) {
	state = State::HOVER;
	request_redraw();
}
void Button::on_mouse_leave(const vec2&) {
	state = State::DEFAULT;
	request_redraw();
}

void Button::get_content_min_size(int &w, int &h) {
	if (text_w < 0) {
		font::set_font(Theme::_default.font_name, Theme::_default.font_size);
		auto dim = font::get_text_dimensions(title);
		text_w = int(dim.bounding_width);
		text_h = int(dim.inner_height());
	}
	w = text_w + Theme::_default.button_margin_x * 2;
	h = text_h + Theme::_default.button_margin_y * 2;
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
	p->draw_str({_area.center().x - dim.bounding_width / ui_scale / 2, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
}

}

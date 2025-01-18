#include "CheckBox.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

	CheckBox::CheckBox(const string &_id, const string &t) : Label(_id, t) {
		can_grab_focus = true;
		expand_x = true;
		expand_y = false;
	}

	void CheckBox::on_left_button_down(const vec2&) {
		state = State::PRESSED;
		request_redraw();
	}
	void CheckBox::on_left_button_up(const vec2&) {
		state = State::HOVER;
		checked = !checked;
		request_redraw();

		if (owner)
			owner->handle_event(id, "hui:click", true);
	}
	void CheckBox::on_mouse_enter(const vec2&) {
		state = State::HOVER;
		request_redraw();
	}
	void CheckBox::on_mouse_leave(const vec2&) {
		state = State::DEFAULT;
		request_redraw();
	}

	void CheckBox::get_content_min_size(int &w, int &h) {
		if (text_w < 0) {
			font::set_font(Theme::_default.font_name, Theme::_default.font_size * ui_scale);
			auto dim = font::get_text_dimensions(title);
			text_w = int(dim.bounding_width / ui_scale);
			text_h = int(dim.inner_height() / ui_scale);
		}
		w = text_w + Theme::_default.button_margin_x * 2;
		h = text_h + Theme::_default.button_margin_y * 2;
	}

	void CheckBox::_draw(Painter *p) {
		color bg = Theme::_default.background_button;
		if (checked) {
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
		p->set_fill(checked);
		p->draw_rect({_area.x1 + 4, _area.x1 + 29, _area.center().y - 12, _area.center().y + 12});
		p->set_roundness(0);

		if (checked) {
			p->set_color(Theme::_default.text);
			p->set_line_width(4);
			float ym = _area.center().y;
			p->draw_lines({{_area.x1 + 10, ym}, {_area.x1 + 16, ym + 6}, {_area.x1 + 22, ym - 6}});
			p->set_line_width(1);
		}

		p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
		auto dim = font::get_text_dimensions(title);

		p->set_color(Theme::_default.text);
		p->draw_str({_area.x1 + 32, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
		p->set_fill(true);
	}

}

#include "CheckBox.h"
#include "../Painter.h"
#include "../draw/font.h"
#include "../Theme.h"

namespace xhui {

	CheckBox::CheckBox(const string &_id, const string &t) : Label(_id, t) {
		state = State::DEFAULT;
		can_grab_focus = true;
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Shrink;
	}

void CheckBox::check(bool _checked) {
	checked = _checked;
	request_redraw();
}

bool CheckBox::is_checked() {
	return checked;
}

	void CheckBox::on_left_button_down(const vec2&) {
		state = State::PRESSED;
		request_redraw();
	}
	void CheckBox::on_left_button_up(const vec2&) {
		state = State::HOVER;
		checked = !checked;
		emit_event(event_id::Changed, true);
		request_redraw();
	}
	void CheckBox::on_mouse_enter(const vec2&) {
		state = State::HOVER;
		request_redraw();
	}
	void CheckBox::on_mouse_leave(const vec2&) {
		state = State::DEFAULT;
		request_redraw();
	}

	void CheckBox::get_content_min_size(int &w, int &h) const {
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
		rect box = {_area.x1 + 7, _area.x1 + 28, _area.center().y - 11, _area.center().y + 11};
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(box);
		if (!checked) {
			p->set_color(Theme::_default.background);
			p->set_roundness(Theme::_default.button_radius-2);
			p->draw_rect(box.grow(-2));
		}
		p->set_roundness(0);

		if (checked) {
			p->set_color(Theme::_default.text);
			p->set_line_width(4);
			float ym = _area.center().y;
			p->draw_lines({{_area.x1 + 11, ym}, {_area.x1 + 17, ym + 6}, {_area.x1 + 23, ym - 6}});
			p->set_line_width(1);
		}

		p->set_font(Theme::_default.font_name, Theme::_default.font_size, false, false);
		auto dim = font::get_text_dimensions(title);

		p->set_color(Theme::_default.text);
		p->draw_str({box.x2 + 6, _area.center().y - dim.inner_height() / ui_scale / 2}, title);
		p->set_fill(true);
	}

}

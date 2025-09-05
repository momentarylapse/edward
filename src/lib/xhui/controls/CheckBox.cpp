#include "CheckBox.h"
#include "../Painter.h"
#include <lib/ygraphics/font.h>
#include "../Theme.h"

namespace xhui {

CheckBox::CheckBox(const string &_id, const string &t) :
		Control(_id),
		label(_id + ":label", t)
{
	state = State::DEFAULT;
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
	label.margin.x1 = label.margin.x2 = Theme::_default.button_margin_x;
	label.margin.y1 = label.margin.y2 = Theme::_default.button_margin_y;
}

Array<Control*> CheckBox::get_children(ChildFilter f) const {
	return {static_cast<Control*>(const_cast<Label*>(&label))};
}

void CheckBox::check(bool _checked) {
	checked = _checked;
	request_redraw();
}

bool CheckBox::is_checked() {
	return checked;
}

void CheckBox::on_left_button_down(const vec2&) {
	if (enabled)
		state = State::PRESSED;
	request_redraw();
}
void CheckBox::on_left_button_up(const vec2&) {
	if (enabled) {
		state = State::HOVER;
		checked = !checked;
		emit_event(event_id::Changed, true);
	}
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

vec2 CheckBox::get_content_min_size() const {
	return label.get_content_min_size() + vec2(22, 0);
}

void CheckBox::negotiate_area(const rect& available) {
	Control::negotiate_area(available);
	label.negotiate_area({_area.p00() + vec2(20, 0), _area.p11()});
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

	label._draw(p);
}

}

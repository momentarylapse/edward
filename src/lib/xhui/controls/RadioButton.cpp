#include "RadioButton.h"
#include "../Painter.h"
#include <lib/ygraphics/font.h>
#include "../Theme.h"

namespace xhui {

RadioButton::RadioButton(const string &_id, const string &t) :
		CheckBox(_id, t)
{
}

void RadioButton::check(bool _checked) {
	checked = _checked;
	request_redraw();
}

void RadioButton::on_left_button_down(const vec2&) {
	state = State::PRESSED;
	request_redraw();
}
void RadioButton::on_left_button_up(const vec2&) {
	state = State::HOVER;
	if (!checked) {
		checked = true;
		emit_event(event_id::Changed, true);

		for (auto g: group())
			if (g != this) {
				g->checked = false;
				g->emit_event(event_id::Changed, true);
			}
	}
	request_redraw();
}

Array<CheckBox*> RadioButton::group() const {
	if (!owner)
		return {};
	string prefix = id.explode(":")[0] + ":";
	Array<CheckBox*> r;
	for (auto c: owner->controls)
		if (c->id.head(prefix.num) == prefix)
			r.add(static_cast<CheckBox*>(c));
	return r;
}


void RadioButton::_draw(Painter *p) {
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
	p->draw_circle(box.center(), 10);
	if (!checked) {
		p->set_color(Theme::_default.background);
		p->draw_circle(box.center(), 8);
	}

	if (checked) {
		p->set_color(Theme::_default.text);
		p->draw_circle(box.center(), 5);
	}

	label._draw(p);
}

}

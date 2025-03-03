//
// Created by Michael Ankele on 2025-03-03.
//

#include "ToggleButton.h"

#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>

namespace xhui {

ToggleButton::ToggleButton(const string& id, const string& title) : Button(id, title) {
	checked = false;
}

void ToggleButton::check(bool _checked) {
	checked = _checked;
	request_redraw();
}

bool ToggleButton::is_checked() {
	return checked;
}

void ToggleButton::on_click() {
	check(!checked);
	emit_event(event_id::Changed, true);
}

void ToggleButton::_draw(Painter* p) {
	color bg = Theme::_default.background_button;
	if (checked) {
		if (state == State::HOVER) {
			bg = Theme::_default.background_active;
		} else if (state == State::PRESSED) {
			bg = Theme::_default.background_active;
		} else {
			bg = Theme::_default.background_active;
		}
	} else {
		if (state == State::HOVER) {
			bg = Theme::_default.background_hover;
		} else if (state == State::PRESSED) {
			bg = Theme::_default.background_active;
		}
	}
	if (primary or state != State::DEFAULT or !flat or checked) {
		p->set_color(bg);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);
	}

	label._draw(p);
}




} // xhui
//
// Created by Michael Ankele on 2025-02-03.
//

#include "ColorButton.h"
#include "../dialogs/ColorSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {
ColorButton::ColorButton(const string& id) : Button(id, "") {
}

color ColorButton::get_color() {
	return _color;
}

void ColorButton::set_color(const color& c) {
	_color = c;
	request_redraw();
}

void ColorButton::on_click() {
	ColorSelectionDialog::ask(owner, "Pick a color", _color, {}).then([this] (const color& c) {
		set_color(c);
		emit_event(event_id::Changed, true);
	});
}


void ColorButton::_draw(Painter* p) {

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
	p->set_roundness(Theme::_default.button_radius);
	p->set_color(bg);
	p->draw_rect(_area);
	p->set_color(_color);
	p->draw_rect(_area.grow(-7));
	p->set_roundness(0);
}




} // xhui
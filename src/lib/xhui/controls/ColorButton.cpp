//
// Created by Michael Ankele on 2025-02-03.
//

#include "ColorButton.h"
#include "../dialogs/ColorSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

void draw_checkerboard(Painter* p, const rect& area);

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
	Array<string> params;
	if (with_alpha)
		params.add("alpha");
	ColorSelectionDialog::ask(owner, "Pick a color", _color, params).then([this] (const color& c) {
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
	if (_color.a < 1) {
		p->set_roundness(0);
		draw_checkerboard(p, _area.grow(-5));
		p->set_roundness(Theme::_default.button_radius);
	}
	p->set_color(_color);
	p->draw_rect(_area.grow(-5));
	p->set_roundness(0);
}

void ColorButton::set_option(const string& key, const string& value) {
	if (key == "withalpha" or key == "alpha") {
		with_alpha = true;
	} else {
		Button::set_option(key, value);
	}
}




} // xhui
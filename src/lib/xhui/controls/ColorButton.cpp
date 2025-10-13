//
// Created by Michael Ankele on 2025-02-03.
//

#include "ColorButton.h"
#include "../dialogs/ColorSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

color color_convert(const color& c, ColorSpace from, ColorSpace to) {
	if (from == to)
		return c;
	if (from == ColorSpace::Linear and to == ColorSpace::SRGB)
		return c.linear_to_srgb();
	if (from == ColorSpace::SRGB and to == ColorSpace::Linear)
		return c.srgb_to_linear();
	return c;
}

color ColorButton::color_to_user(const color &c) const {
	return color_convert(c, color_space_display, color_space_user);
}

color ColorButton::color_from_user(const color &c) const {
	return color_convert(c, color_space_user, color_space_display);
}

void draw_checkerboard(Painter* p, const rect& area);

ColorButton::ColorButton(const string& id) : Button(id, "") {
	color_space_user = ColorSpace::SRGB;
}

color ColorButton::get_color() {
	return color_to_user(_color);
}

void ColorButton::set_color(const color& c) {
	_color = color_from_user(c);
	if (!with_alpha)
		_color.a = 1;
	request_redraw();
}

void ColorButton::on_click() {
	Array<string> params;
	if (with_alpha)
		params.add("alpha");
	if (color_space_user == ColorSpace::Linear)
		params.add("linear");

	// using "internal" (srgb) colors here:
	ColorSelectionDialog::ask(owner, "Pick a color", _color, params).then([this] (const color& c) {
		_color = c;
		request_redraw();
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
	} else if (key == "linear") {
		color_space_user = ColorSpace::Linear;
	} else {
		Button::set_option(key, value);
	}
}




} // xhui
//
// Created by Michael Ankele on 2025-04-16.
//

#include "Separator.h"

#include <lib/xhui/Painter.h>
#include <lib/xhui/Theme.h>

namespace xhui {

Separator::Separator(const string& id, Orientation _orientation) : Control(id) {
	orientation = _orientation;
}

vec2 Separator::get_content_min_size() const {
	return {10, 10};
}

vec2 Separator::get_greed_factor() const {
	return {0,0};
}

void Separator::_draw(Painter* p) {
	p->set_color(Theme::_default.background_button);
	p->set_line_width(3);
	if (orientation == Orientation::HORIZONTAL) {
		float y = _area.center().y;
		p->draw_line({_area.x1 + 8, y}, {_area.x2 - 8, y});
	} else {
		float x = _area.center().x;
		p->draw_line({x, _area.y1 + 8}, {x, _area.y2 - 8});
	}
}

void Separator::set_option(const string &key, const string &value) {
	if (key == "horizontal") {
		orientation = Orientation::HORIZONTAL;
		request_redraw();
	} else if (key == "vertical") {
		orientation = Orientation::VERTICAL;
		request_redraw();
	} else {
		Control::set_option(key, value);
	}
}


} // xhui
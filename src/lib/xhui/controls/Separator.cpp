//
// Created by Michael Ankele on 2025-04-16.
//

#include "Separator.h"
#include <lib/xhui/Painter.h>
#include <lib/xhui/Theme.h>

namespace xhui {

Separator::Separator(const string& id, Orientation _orientation) : Control(id) {
	orientation = _orientation;
	size_mode_x = (orientation == Orientation::HORIZONTAL) ? SizeMode::Fill : SizeMode::Shrink;
	size_mode_y = (orientation == Orientation::HORIZONTAL) ? SizeMode::Shrink : SizeMode::Fill;
}

vec2 Separator::get_content_min_size() const {
	return {10, 10};
}

void Separator::_draw(Painter* p) {
	p->set_color(Theme::_default.background_button);
	p->set_line_width(2);
	if (orientation == Orientation::HORIZONTAL) {
		float y = area.center().y;
		p->draw_line({area.x1, y}, {area.x2, y});
	} else {
		float x = area.center().x;
		p->draw_line({x, area.y1 + 8}, {x, area.y2 - 8});
	}
}

void Separator::set_option(const string &key, const string &value) {
	if (key == "horizontal") {
		orientation = Orientation::HORIZONTAL;
		size_mode_x = SizeMode::Fill;
		size_mode_y = SizeMode::Shrink;
		request_redraw();
	} else if (key == "vertical") {
		orientation = Orientation::VERTICAL;
		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Fill;
		request_redraw();
	} else {
		Control::set_option(key, value);
	}
}


} // xhui
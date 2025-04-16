//
// Created by Michael Ankele on 2025-04-16.
//

#include "Separator.h"

#include <lib/xhui/Painter.h>
#include <lib/xhui/Theme.h>

namespace xhui {

Separator::Separator(const string& id) : Control(id) {}

vec2 Separator::get_content_min_size() const {
	return {10, 10};
}

vec2 Separator::get_greed_factor() const {
	return {0,0};
}

void Separator::_draw(Painter* p) {
	p->set_color(Theme::_default.background_button);
	p->set_line_width(3);
	float y = _area.center().y;
	p->draw_line({_area.x1, y}, {_area.x2, y});
}

} // xhui
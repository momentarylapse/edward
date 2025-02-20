//
// Created by Michael Ankele on 2025-01-20.
//

#include "Mode.h"

#include <lib/image/Painter.h>
#include <lib/xhui/Theme.h>
#include <renderer/regions/RegionRenderer.h>

Mode::Mode(Session* _session) {
	session = _session;
	multi_view = nullptr;
}

void Mode::draw_info(Painter* p, const string& msg) {
	vec2 pos = p->area().p01() + vec2(30, -40);
	vec2 size = p->get_str_size(msg);
	p->set_color(xhui::Theme::_default.background_button);
	p->set_roundness(7);
	p->draw_rect(rect(pos, pos + size).grow(10));
	p->set_color(xhui::Theme::_default.text_label);
	p->set_roundness(0);
	p->draw_str(pos, msg);
}




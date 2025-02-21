//
// Created by Michael Ankele on 2025-01-20.
//

#include "Mode.h"

#include <Session.h>
#include <lib/image/Painter.h>
#include <lib/xhui/Theme.h>
#include <renderer/regions/RegionRenderer.h>

#include "DrawingHelper.h"

Mode::Mode(Session* _session) {
	session = _session;
	multi_view = nullptr;
}

void Mode::draw_info(Painter* p, const string& msg) {
	vec2 pos = p->area().p01() + vec2(30, -40);
	session->drawing_helper->draw_boxed_str(p, pos, msg);
}




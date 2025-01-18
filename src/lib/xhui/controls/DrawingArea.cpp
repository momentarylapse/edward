#include "DrawingArea.h"
#include "../Painter.h"

namespace xhui {

DrawingArea::DrawingArea(const string &_id) : Control(_id) {
	expand_x = true;
	expand_y = true;
}


void DrawingArea::_draw(Painter *p) {
	// backup
	int w = p->width;
	int h = p->height;
	auto old_area = p->_area;
	auto old_native_area = p->native_area;
	// new config
	p->width = _area.width();
	p->height = _area.height();
	p->_area = _area;
	p->native_area = {_area.x1 * ui_scale, _area.x2 * ui_scale, _area.y1 * ui_scale, _area.y2 * ui_scale};

	//p->set_clip(_area);
	p->set_transform({}, vec2(_area.x1, _area.y1));
	if (owner) {
		if (first_draw)
			owner->handle_event_p(id, "hui:initialize", p);
		first_draw = false;
		owner->handle_event_p(id, "hui:draw", p);
	}
	p->set_transform({}, vec2(0, 0));

	// restore
	p->width = w;
	p->height = h;
	p->_area = old_area;
	p->native_area = old_native_area;
	//p->set_clip(p->area());
}

}

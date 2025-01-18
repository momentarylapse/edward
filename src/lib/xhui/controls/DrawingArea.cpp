#include "DrawingArea.h"
#include "../Painter.h"
#include "../../math/complex.h"

namespace xhui {

DrawingArea::DrawingArea(const string &_id) : Control(_id) {
	expand_x = true;
	expand_y = true;
}


void DrawingArea::_draw(Painter *p) {
	int w = p->width;
	int h = p->height;
	p->width = _area.width();
	p->height = _area.height();
	//p->set_clip(_area);
	p->set_transform({}, vec2(_area.x1, _area.y1));
	if (owner)
		owner->handle_event_p(id, "hui:draw", p);
	p->set_transform({}, vec2(0, 0));
	p->width = w;
	p->height = h;
	//p->set_clip(p->area());
}

}

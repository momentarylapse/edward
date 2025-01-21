#include "DrawingArea.h"

#include <lib/os/msg.h>

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
			owner->handle_event_p(id, event_id::Initialize, p);
		first_draw = false;
		owner->handle_event_p(id, event_id::Draw, p);
	}
	p->set_transform({}, vec2(0, 0));

	// restore
	p->width = w;
	p->height = h;
	p->_area = old_area;
	p->native_area = old_native_area;
	//p->set_clip(p->area());
}
void DrawingArea::on_left_button_down(const vec2& m) {
	if (owner)
		owner->handle_event(id, event_id::LeftButtonDown, false);
}
void DrawingArea::on_left_button_up(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:left-button-up", false);
}
void DrawingArea::on_middle_button_down(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:middle-button-down", false);
}
void DrawingArea::on_middle_button_up(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:middle-button-up", false);
}
void DrawingArea::on_right_button_down(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:right-button-down", false);
}
void DrawingArea::on_right_button_up(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:right-button-up", false);
}
void DrawingArea::on_mouse_move(const vec2& m, const vec2& d) {
	if (owner)
		owner->handle_event(id, event_id::MouseMove, false);
}
void DrawingArea::on_mouse_enter(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:mouse-enter", false);
}
void DrawingArea::on_mouse_leave(const vec2& m) {
	if (owner)
		owner->handle_event(id, "hui:mouse-leave", false);
}
void DrawingArea::on_mouse_wheel(const vec2& d) {
	if (owner)
		owner->handle_event(id, "hui:mouse-wheel", false);
}
void DrawingArea::on_key_down(int key) {
	if (owner)
		owner->handle_event(id, "hui:key-down", false);
}
void DrawingArea::on_key_up(int key) {
	if (owner)
		owner->handle_event(id, "hui:key-up", false);
}

}

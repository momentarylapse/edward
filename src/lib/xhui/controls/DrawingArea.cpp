#include "DrawingArea.h"
#include <lib/os/msg.h>
#include "../Painter.h"


namespace xhui {

DrawingArea::DrawingArea(const string &_id) : Control(_id) {
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Expand;
}


void DrawingArea::_draw(Painter *p) {
	if (owner) {
		for_painter_do(p, [this] (Painter* p) {
				if (first_draw)
					owner->handle_event_p(id, event_id::Initialize, p);
				first_draw = false;
				owner->handle_event_p(id, event_id::Draw, p);
		});
	}
}

void DrawingArea::for_painter_do(Painter* p, std::function<void(Painter*)> f) {
	// backup
	int w = p->width;
	int h = p->height;
	const auto old_area = p->_area;
	const auto old_native_area = p->native_area;
	const auto old_clip = p->clip();
	// new config
	p->width = (int)area.width();
	p->height = (int)area.height();
	p->_area = area;
	p->native_area = {area.x1 * p->ui_scale.x, area.x2 * p->ui_scale.x, area.y1 * p->ui_scale.y, area.y2 * p->ui_scale.y};

	p->set_clip(area);
	//p->set_transform({}, vec2(area.x1, area.y1)); // NOPE, we use window coordinates!
	f(p);
	//p->set_transform({}, vec2(0, 0));

	// restore
	p->width = w;
	p->height = h;
	p->_area = old_area;
	p->native_area = old_native_area;
	p->set_clip(old_clip);

}
void DrawingArea::on_left_button_down(const vec2& m) {
	emit_event(event_id::LeftButtonDown, false);
}
void DrawingArea::on_left_button_up(const vec2& m) {
	emit_event(event_id::LeftButtonUp, false);
}
void DrawingArea::on_middle_button_down(const vec2& m) {
	emit_event(event_id::MiddleButtonDown, false);
}
void DrawingArea::on_middle_button_up(const vec2& m) {
	emit_event(event_id::MiddleButtonUp, false);
}
void DrawingArea::on_right_button_down(const vec2& m) {
	emit_event(event_id::RightButtonDown, false);
}
void DrawingArea::on_right_button_up(const vec2& m) {
	emit_event(event_id::RightButtonUp, false);
}
void DrawingArea::on_mouse_move(const vec2& m, const vec2& d) {
	emit_event(event_id::MouseMove, false);
}
void DrawingArea::on_mouse_enter(const vec2& m) {
	emit_event(event_id::MouseEnter, false);
}
void DrawingArea::on_mouse_leave(const vec2& m) {
	emit_event(event_id::MouseLeave, false);
}
void DrawingArea::on_mouse_wheel(const vec2& d) {
	emit_event(event_id::MouseWheel, false);
}
void DrawingArea::on_key_down(int key) {
	emit_event(event_id::KeyDown, false);
}
void DrawingArea::on_key_up(int key) {
	emit_event(event_id::KeyUp, false);
}
void DrawingArea::on_key_char(int character) {
	emit_event(event_id::KeyChar, false);
}

}

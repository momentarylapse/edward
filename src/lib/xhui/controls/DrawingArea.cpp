#include "DrawingArea.h"

#include <lib/nix/nix.h>
#include <lib/nix/nix_view.h>
#include <lib/os/msg.h>

#include "../Painter.h"

#if HAS_LIB_GL
namespace nix {
	mat4 create_pixel_projection_matrix();
}
#endif

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


#if HAS_LIB_GL
	nix::set_projection_matrix(nix::create_pixel_projection_matrix() * mat4::translation({0,0,0.5f}) * mat4::scale(p->ui_scale, p->ui_scale, 1));
	nix::set_view_matrix(mat4::ID);
	nix::set_model_matrix(mat4::ID);
	//nix::clear(color(1, 0.15f, 0.15f, 0.3f));
	nix::set_cull(nix::CullMode::NONE);
	nix::set_z(false, false);
#endif
}

void DrawingArea::for_painter_do(Painter* p, std::function<void(Painter*)> f) {

	// backup
	int w = p->width;
	int h = p->height;
	auto old_area = p->_area;
	auto old_native_area = p->native_area;
	// new config
	p->width = _area.width();
	p->height = _area.height();
	p->_area = _area;
	p->native_area = {_area.x1 * p->ui_scale, _area.x2 * p->ui_scale, _area.y1 * p->ui_scale, _area.y2 * p->ui_scale};

	//p->set_clip(_area);
	p->set_transform({}, vec2(_area.x1, _area.y1));
	f(p);
	p->set_transform({}, vec2(0, 0));

	// restore
	p->width = w;
	p->height = h;
	p->_area = old_area;
	p->native_area = old_native_area;
	//p->set_clip(p->area());

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

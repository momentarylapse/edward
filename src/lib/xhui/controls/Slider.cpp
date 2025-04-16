//
// Created by Michael Ankele on 2025-04-16.
//

#include "Slider.h"

#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>

namespace xhui {

Slider::Slider(const string& id) : Control(id) {
	can_grab_focus = true;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;
	state = State::DEFAULT;
	min = 0;
	max = 1;
	step = 0;
	button_radius = 7;
}


float Slider::get_float() {
	return value;
}

void Slider::set_float(float f) {
	value = f;
	request_redraw();
}

void Slider::on_left_button_down(const vec2& m) {
	if (enabled) {
		state = State::PRESSED;
		value = mouse_to_value(m);
		emit_event(event_id::Changed, true);
	}
	request_redraw();
	emit_event(event_id::LeftButtonDown, false);
}

void Slider::on_left_button_up(const vec2&) {
	if (enabled)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::LeftButtonUp, false);
	//if (enabled)
	//	on_click();
}

void Slider::on_mouse_move(const vec2& m, const vec2& d) {
	if (state == State::PRESSED) {
		value = mouse_to_value(m);
		request_redraw();
		emit_event(event_id::Changed, true);
	}
}

void Slider::on_mouse_wheel(const vec2& d) {
	value = clamp(value + (d.x - d.y) * 0.05f * (max - min), min, max);
	request_redraw();
	emit_event(event_id::Changed, true);
}


float Slider::mouse_to_value(const vec2& m) const {
	float v = (m.x - _area.x1 - button_radius) / (_area.width() - button_radius*2);
	v = clamp(v, 0.0f, 1.0f);
	return min + v * (max - min);
}


void Slider::on_mouse_enter(const vec2&) {
	if (enabled)
		state = State::HOVER;
	request_redraw();
	emit_event(event_id::MouseEnter, false);
}

void Slider::on_mouse_leave(const vec2&) {
	if (enabled)
		state = State::DEFAULT;
	request_redraw();
	emit_event(event_id::MouseLeave, false);
}

vec2 Slider::get_content_min_size() const {
	return {button_radius*2,0};
	//return label.get_content_min_size() + padding.p00() + padding.p11();
}

void Slider::negotiate_area(const rect& available) {
	Control::negotiate_area(available);
	//label.negotiate_area({available.p00() + padding.p00(), available.p11() - padding.p11()});
}

void Slider::_draw(Painter* p) {
	p->set_color(Theme::_default.background_button);
	p->set_line_width(3);
	float y = _area.center().y;
	p->draw_line({_area.x1, y}, {_area.x2, y});


	color bg = Theme::_default.background_button;
	if (state == State::HOVER) {
		bg = Theme::_default.background_hover;
	} else if (state == State::PRESSED) {
		bg = Theme::_default.background_active;
	}

	float x = _area.x1 + button_radius + value * (_area.width() - button_radius*2);

	p->set_color(bg);
	p->set_roundness(Theme::_default.button_radius);
	p->draw_rect({x-button_radius, x+button_radius, _area.y1, _area.y2});
	p->set_roundness(0);
}


void Slider::set_option(const string& key, const string& value) {
	if (key == "range") {
		auto xx = value.explode(":");
		if (xx.num >= 1)
			min = xx[0]._float();
		if (xx.num >= 2)
			max = xx[1]._float();
		if (xx.num >= 3)
			step = xx[2]._float();
	} else {
		Control::set_option(key, value);
	}
}



} // xhui
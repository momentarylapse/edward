//
// Created by Michael Ankele on 2025-02-01.
//

#include "SpinButton.h"
#include <lib/xhui/Painter.h>
#include <lib/xhui/Theme.h>
#include <cmath>

namespace xhui {

constexpr float BUTTON_DX = 30;

SpinButton::SpinButton(const string& id, float _value) : Edit(id, "") {
	size_mode_x = SizeMode::Shrink;
	numerical = true;
	hover = Hover::Other;
	pressed = Hover::Other;

	value = _value;
	text_needs_update = true;
	_min = -1000000;
	_max = 1000000;
	step = 1.0f;
	decimals = 0;
}

float SpinButton::get_float() {
	return value;
}

void SpinButton::set_float(float f) {
	value = f;
	text_needs_update = true;
	request_redraw();
}

int SpinButton::get_int() {
	return (int)value;
}

void SpinButton::set_int(int i) {
	set_float((float)i);
}

void SpinButton::on_edit() {
	_update_value_from_text();
	emit_event(event_id::Changed, true);
}

void SpinButton::_update_text_from_value() {
	// TODO decimals
	Edit::set_string(f2s(value, decimals));
	text_needs_update = false;
	request_redraw();
}

vec2 SpinButton::get_content_min_size() const {
	if (owner)
		if (auto win = owner->get_window())
			ui_scale = win->ui_scale;
	auto dims = get_cached_text_dimensions(f2s(_max, decimals), default_font_regular, font_size, ui_scale);
	auto s = Edit::get_content_min_size();
	float w = dims.bounding_width / ui_scale;
	s.x = w + margin_x*2;
	if (show_buttons)
		s.x += BUTTON_DX*2;
	return s;
}


SpinButton::Hover SpinButton::get_hover(const vec2& m) const {
	if (_area.inside(m) and enabled and show_buttons) {
		if (m.x > _area.x2 - BUTTON_DX)
			return Hover::Plus;
		if (m.x > _area.x2 - BUTTON_DX * 2)
			return Hover::Minus;
	}
	return Hover::Other;
}

void SpinButton::_update_value_from_text() {
	value = text._float();
	value = clamp(value, _min, _max);
}

void SpinButton::on_mouse_leave(const vec2& m) {
	hover = Hover::Other;
	Edit::on_mouse_leave(m);
	request_redraw();
}

void SpinButton::on_mouse_move(const vec2& m, const vec2& d) {
	hover = get_hover(m);
	request_redraw();
	if (hover == Hover::Other)
		Edit::on_mouse_move(m, d);
}

void SpinButton::on_left_button_down(const vec2& m) {
	hover = get_hover(m);
	pressed = hover;
	switch (hover) {
	case Hover::Plus:
		value = clamp(value + step, _min, _max);
		_update_text_from_value();
		emit_event(event_id::Changed, true);
		break;
	case Hover::Minus:
		value = clamp(value - step, _min, _max);
		_update_text_from_value();
		emit_event(event_id::Changed, true);
		break;
	case Hover::Other:
		Edit::on_left_button_down(m);
	}
	request_redraw();
}

void SpinButton::on_left_button_up(const vec2& m) {
	if (hover == Hover::Other)
		Edit::on_left_button_up(m);
	pressed = Hover::Other;
	request_redraw();
}

void SpinButton::on_mouse_wheel(const vec2& d) {
	if (!enabled)
		return;
	int n = (int)d.y;
	// TODO better "small" step tweak
	// maybe accumulate?
	/*if (d.y > 0 and d.y < 0.8f)
		n = 1;
	if (d.y < 0 and d.y > -0.8f)
		n = -1;*/
	value = clamp(value + (float)n * step, _min, _max);
	_update_text_from_value();
	emit_event(event_id::Changed, true);
}



void SpinButton::_draw(Painter* p) {
	ui_scale = p->ui_scale;
	if (text_needs_update)
		_update_text_from_value();
	Edit::_draw(p);

	if (!show_buttons)
		return;

	float dx = BUTTON_DX;

	const rect area_plus = {_area.p10() - vec2(dx,0), _area.p11()};
	const rect area_minus = {_area.p10() - vec2(dx*2,0), _area.p11() - vec2(dx,0)};

	if (hover == Hover::Plus or pressed == Hover::Plus) {
		p->set_color(pressed == Hover::Plus ? Theme::_default.background_active : Theme::_default.background_hover);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(area_plus.grow(-2));
	}
	if (hover == Hover::Minus or pressed == Hover::Minus) {
		p->set_color(pressed == Hover::Minus ? Theme::_default.background_active : Theme::_default.background_hover);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(area_minus.grow(-2));
	}

	p->set_color(color::mix(Theme::_default.text_label, Theme::_default.background, 0.8f));
	p->draw_line(area_minus.p00(), area_minus.p01());
	p->draw_line(area_plus.p00(), area_plus.p01());

	p->set_color(Theme::_default.text);
	/*if (!enabled)
		p->set_color(Theme::_default.text_disabled);
	vec2 m = (_area.p10() + _area.p11()) / 2 - vec2(dx/2,0);
	p->set_line_width(2);
	float r = 5;
	p->draw_line(m + vec2(-dx-r,0), m + vec2(-dx + r,0));
	p->draw_line(m + vec2( - r,0), m + vec2(r,0));
	p->draw_line(m + vec2(0,-r), m + vec2(0,r));
	//p->draw_str((_area.p10() + _area.p11()) / 2 - vec2(dx/2,0), "+");
	//p->draw_str((_area.p10() + _area.p11()) / 2 - vec2(dx*3/2,0), "-");
	p->set_line_width(1);*/


	// currently faster to use textures:
	auto im_plus = load_image("hui:plus");
	prepare_image(im_plus);
	p->draw_ximage(area_plus, im_plus);
	auto im_minus = load_image("hui:minus");
	prepare_image(im_minus);
	p->draw_ximage(area_minus, im_minus);
}



void SpinButton::set_option(const string& key, const string& value) {
	if (key == "range") {
		auto x = value.explode(":");
		if (x.num >= 1 and x[0].num > 0)
			_min = x[0]._float();
		if (x.num >= 2 and x[1].num > 0)
			_max = x[1]._float();
		if (x.num >= 3 and x[2].num > 0) {
			step = x[2]._float();
			decimals = max((int)(-log10f(step) + 0.5f), 0);
		}
		request_redraw();
	} else if (key == "compact") {
		show_buttons = false;
	} else {
		Control::set_option(key, value);
	}
}





} // xhui
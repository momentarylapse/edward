//
// Created by michi on 03.02.25.
//

#include "ColorSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"
#include <cmath>

namespace xhui {


void draw_checkerboard(Painter* p, const rect& area) {
	float w = area.width();
	float h = area.height();

	float D = 9;
	for (int i=0; i<=(int)(w / D); i++)
		for (int j=0; j<=(int)(h / D); j++) {
			p->set_color((i%2) != (j%2) ? Black : White);
			p->draw_rect({area.x1 + (float)i*D,
				area.x1 + min((float)(i+1)*D, w),
				area.y1 + (float)j*D,
				area.y1 + min((float)(j+1)*D, h)});
		}
}

class ColorHueSelector : public Control {
public:
	float hue;
	std::function<void(float)> callback;

	explicit ColorHueSelector(const string& id, const std::function<void(float)>& _callback) : Control(id) {
		size_mode_x = SizeMode::Shrink;
		size_mode_y = SizeMode::Expand;
		min_width_user = 30;
		hue = 0;
		callback = _callback;
	}

	void set_float(float value) override {
		hue = value;
		request_redraw();
	}
	float get_float() override {
		return hue;
	}
	void on_left_button_down(const vec2& m) override {
		hue = clamp((m.y - _area.y1) / _area.height(), 0.0f, 1.0f);
		request_redraw();
		if (callback)
			callback(hue);
		emit_event(event_id::Changed, true);
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		if (auto w = get_window())
			if (w->button(0))
				on_left_button_down(m);
	}
	void _draw(Painter* p) override {
		float w = _area.width();
		float h = _area.height();

		const int N = 100;
		for (int i=0; i<N; i++) {
			p->set_color(color::from_hsb((float)i / (float)N, 1, 1, 1));
			p->draw_rect({_area.x1, _area.x2, _area.y1 + (float)i / (float)N * h, _area.y1 + (float)(i + 1) / (float)N * h});
			p->set_color(color::from_hsb(color::from_hsb((float)i / (float)N, 1, 1, 1).hue(), 1, 1, 1));
			p->draw_rect({_area.x1 + w/2, _area.x2, _area.y1 + (float)i / (float)N * h, _area.y1 + (float)(i + 1) / (float)N * h});
		}

		p->set_color(Black);
		p->draw_circle(_area.p00() + vec2(w/2, hue * h), 6);
		p->set_color(White);
		p->draw_circle(_area.p00() + vec2(w/2, hue * h), 4);
	}
};

class ColorSBSelector : public Control {
public:
	float hue, saturation, brightness;
	std::function<void(float, float)> callback;
	explicit ColorSBSelector(const string& id, const std::function<void(float, float)>& _callback) : Control(id) {
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
		hue = 0;
		saturation = 0;
		brightness = 0;
		callback = _callback;
	}

	void set_hue(float h) {
		hue = h;
		request_redraw();
	}
	void set_sb(float s, float b) {
		saturation = s;
		brightness = b;
		request_redraw();
	}
	void on_left_button_down(const vec2& m) override {
		saturation = clamp((m.x - _area.x1) / _area.width(), 0.0f, 1.0f);
		brightness = clamp(1 - (m.y - _area.y1) / _area.height(), 0.0f, 1.0f);
		request_redraw();
		emit_event(event_id::Changed, true);
		if (callback)
			callback(saturation, brightness);
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		if (auto w = get_window())
			if (w->button(0))
				on_left_button_down(m);
	}
	void _draw(Painter* p) override {
		float w = _area.width();
		float h = _area.height();

		const int N = 100;
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++) {
				p->set_color(color::from_hsb(hue, (float)i / (float)N, 1 - (float)j / (float)N, 1));

				p->draw_rect({_area.x1 + (float)i / (float)N * w, _area.x1 + (float)(i + 1) / (float)N * w, _area.y1 + (float)j / (float)N * h, _area.y1 + (float)(j + 1) / (float)N * h});
			}
		}

		p->set_color(Black);
		p->draw_circle(_area.p00() + vec2(saturation * w, (1 - brightness) * h), 6);
		p->set_color(White);
		p->draw_circle(_area.p00() + vec2(saturation * w, (1 - brightness) * h), 4);
	}
};

class ColorHSBWheel : public Control {
public:
	float hue, saturation, brightness;
	std::function<void(const color&)> callback;
	enum class MouseMode {
		None,
		Hue,
		SaturationBrightness
	} mouse_mode;

	static constexpr float R_CIRCLE = 0.94f;
	static constexpr float R_TRIANGLE = 0.86f;

	explicit ColorHSBWheel(const string& id, const std::function<void(const color&)>& _callback) : Control(id) {
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Expand;
		hue = 0;
		saturation = 0;
		brightness = 0;
		mouse_mode = MouseMode::None;
		callback = _callback;
	}

	void set(const color& c) {
		hue = c.hue();
		saturation = c.saturation();
		brightness = c.hsb_brightness();
		request_redraw();
	}
	color get() const {
		return color::from_hsb(hue, saturation, brightness);
	}
	float radius() const {
		float w = _area.width();
		float h = _area.height();
		return min(w, h) / 2;
	}
	vec2 hue_circle_pos(float _hue, float r) const {
		const float R = radius();
		const vec2 m = _area.center();

		const float phi = _hue * 2 * pi;
		return m + vec2(cosf(phi), sinf(phi)) * R * r;
	}
	void hue_from_mouse(const vec2& m) {
		const vec2 center = _area.center();
		const vec2 d = m - center;
		hue = atan2f(d.y, d.x) / (2 * pi);
		if (hue < 0)
			hue += 1;
	}
	void saturation_brightness_from_mouse(const vec2& m) {
		const vec2 pos0 = hue_circle_pos(hue, R_TRIANGLE);
		const vec2 pos1 = hue_circle_pos(hue + 1.0f / 3, R_TRIANGLE);
		const vec2 pos2 = hue_circle_pos(hue + 2.0f / 3, R_TRIANGLE);
		const vec2 fg = vec2::bary_centric(m, pos0, pos1, pos2);
		// m = pos0 + f (1 - 0) + g (2 - 0)
		saturation = clamp(1 - fg.x / (1 - fg.y), 0.0f, 1.0f);
		brightness = clamp(1 - fg.y, 0.0f, 1.0f);
	}
	void on_left_button_down(const vec2& m) override {
		const float R = radius();
		const vec2 center = _area.center();
		const vec2 d = m - center;
		const float r = d.length();
		mouse_mode = MouseMode::None;

		if (r > R * 0.88f) {
			mouse_mode = MouseMode::Hue;
			hue_from_mouse(m);
		} else {
			mouse_mode = MouseMode::SaturationBrightness;
			saturation_brightness_from_mouse(m);
		}

		emit_event(event_id::Changed, true);
		if (callback)
			callback(get());
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		if (auto w = get_window())
			if (w->button(0)) {
				if (mouse_mode == MouseMode::Hue) {
					hue_from_mouse(m);
				} else if (mouse_mode == MouseMode::SaturationBrightness) {
					saturation_brightness_from_mouse(m);
				}
				request_redraw();
			}

		emit_event(event_id::Changed, true);
		if (callback)
			callback(get());
	}
	void draw_hue_circle(Painter* p) const {
		float R = radius();
		const vec2 m = _area.center();

		auto pos = [&m, R] (float t) {
			float phi = t * 2 * pi;
			return m + vec2(cosf(phi), sinf(phi)) * R * 0.94f;
		};

		const int N = 500;
		for (int i=0; i<N; i++) {
			p->set_color(color::from_hsb((float)i / (float)N, 1, 1));
			p->draw_circle(pos((float)i / (float)N), R * 0.06f);
		}
		p->set_color(Black);
		p->draw_circle(pos(hue), 6);
		p->set_color(White);
		p->draw_circle(pos(hue), 4);
	}
	void draw_sb_triangle(Painter* p) const {
		const float R = radius();

		const vec2 pos0 = hue_circle_pos(hue, R_TRIANGLE);
		const vec2 pos1 = hue_circle_pos(hue + 1.0f / 3, R_TRIANGLE);
		const vec2 pos2 = hue_circle_pos(hue + 2.0f / 3, R_TRIANGLE);
		const vec2 pos0b = hue_circle_pos(hue, R_TRIANGLE + 0.02f);
		const vec2 pos1b = hue_circle_pos(hue + 1.0f / 3, R_TRIANGLE + 0.02f);
		const vec2 pos2b = hue_circle_pos(hue + 2.0f / 3, R_TRIANGLE + 0.02f);

		// (1,1) -> pos0
		// (0,1) -> pos1
		// (x,0) -> pos2
		auto pos = [&pos0, &pos1, &pos2] (float s, float b) {
			vec2 a = pos1 + s * (pos0 - pos1);
			return pos2 + b * (a - pos2);
		};

		const int N = 100;
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++) {
				float s = (float)i / (float)N;
				float b = (float)j / (float)N;
				p->set_color(color::from_hsb(hue, s, b));
				p->draw_circle(pos(s, b), R * 0.02f);
			}
		}

		// border
		p->set_color(White);
		p->set_line_width(4.0f);
		p->draw_lines({pos0b, pos1b, pos2b, pos0b});

		// marker
		p->set_color(Black);
		p->draw_circle(pos(saturation, brightness), 6);
		p->set_color(White);
		p->draw_circle(pos(saturation, brightness), 4);
	}
	void _draw(Painter* p) override {
		draw_hue_circle(p);
		draw_sb_triangle(p);
	}
};

class ColorAlphaSelector : public Control {
public:
	color col;
	float alpha;
	std::function<void(float)> callback;

	explicit ColorAlphaSelector(const string& id, const std::function<void(float)>& _callback) : Control(id) {
		size_mode_x = SizeMode::Expand;
		size_mode_y = SizeMode::Shrink;
		min_height_user = 30;
		alpha = 1;
		col = White;
		callback = _callback;
	}

	void set_float(float value) override {
		alpha = value;
		request_redraw();
	}
	float get_float() override {
		return alpha;
	}
	void set_color(const color& c) override {
		col = c;
		request_redraw();
	}
	void on_left_button_down(const vec2& m) override {
		alpha = clamp((m.x - _area.x1) / _area.width(), 0.0f, 1.0f);
		request_redraw();
		if (callback)
			callback(alpha);
		emit_event(event_id::Changed, true);
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		if (auto w = get_window())
			if (w->button(0))
				on_left_button_down(m);
	}
	void _draw(Painter* p) override {
		float w = _area.width();
		float h = _area.height();

		draw_checkerboard(p, _area);

		const int N = 100;
		for (int i=0; i<N; i++) {
			p->set_color(col.with_alpha((float)i / (float)N));
			p->draw_rect({_area.x1 + (float)i / (float)N * w, _area.x1 + (float)(i + 1) / (float)N * w, _area.y1, _area.y2});
		}

		p->set_color(Black);
		p->draw_circle(_area.p00() + vec2(alpha * w, h/2), 6);
		p->set_color(White);
		p->draw_circle(_area.p00() + vec2(alpha * w, h/2), 4);
	}
};

class ColorHSBSelector : public Grid {
public:
	ColorHSBWheel* hsb_selector;
	ColorAlphaSelector* alpha_selector = nullptr;

	explicit ColorHSBSelector(const string& id) : Grid(id) {
		hsb_selector = new ColorHSBWheel("hsb", [this] (const color& c) {
			if (alpha_selector)
				alpha_selector->set_color(c);
			emit_event(event_id::Changed, true);
		});
		Grid::add_child(hsb_selector, 0, 0);
	}

	void set_color(const color& c) override {
		hsb_selector->set(c);
		if (alpha_selector)
			alpha_selector->set_float(c.a);
	}
	color get_color_no_alpha() const {
		return hsb_selector->get();
	}
	color get_color() override {
		float alpha = 1;
		if (alpha_selector)
			alpha = alpha_selector->get_float();
		return get_color_no_alpha().with_alpha(alpha);
	}
	void set_option(const string& key, const string& value) override {
		if (key == "alpha") {
			alpha_selector = new ColorAlphaSelector("alpha", [this] (float) {
				emit_event(event_id::Changed, true);
			});
			Grid::add_child(alpha_selector, 0, 1);
		} else {
			Grid::set_option(key, value);
		}
	}
};

class ColorIndicator : public Control {
public:
	explicit ColorIndicator(const string& id) : Control(id) {
		col = Black;
	}
	color col;
	void set_color(const color& c) override {
		col = c;
		request_redraw();
	}
	void _draw(Painter* p) override {
		if (col.a < 1)
			draw_checkerboard(p, _area);
		p->set_color(col);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);
	}
};

ColorSelectionDialog::ColorSelectionDialog(Panel* parent, const string& title, const color& col, const Array<string>& params) : Dialog(title, 600, 600, parent) {
	selector = new ColorHSBSelector("selector");
	if (sa_contains(params, "alpha"))
		selector->set_option("alpha", "");
	selector->set_color(col);

	auto g1 = new Grid("grid1");
	Dialog::add_child(g1);
	auto g2 = new Grid("grid2");
	g1->add_child(g2, 0, 0);
	g1->add_child(selector, 0, 1);

	auto g3 = new Grid("grid3");
	g1->add_child(g3, 0, 2);
	g3->add_child(new Label("spacer", ">"), 1, 0);
	auto spacer = new Label("spacer", "");
	g3->add_child(spacer, 3, 0);
	spacer->size_mode_x = SizeMode::Expand;
	indicator_original = new ColorIndicator("indicator-original");
	indicator_original->set_color(col);
	indicator_original->min_width_user = 100;
	indicator_original->size_mode_x = SizeMode::Shrink;
	indicator_original->size_mode_y = SizeMode::Shrink;
	g3->add_child(indicator_original, 0, 0);
	indicator = new ColorIndicator("indicator");
	indicator->set_color(col);
	indicator->min_width_user = 100;
	indicator->size_mode_x = SizeMode::Shrink;
	indicator->size_mode_y = SizeMode::Shrink;
	g3->add_child(indicator, 2, 0);
	auto button_cancel = new Button("cancel", "Cancel");
	button_cancel->min_width_user = 100;
	button_cancel->size_mode_x = SizeMode::Shrink;
	g3->add_child(button_cancel, 4, 0);
	auto button_ok = new Button("ok", "Ok");
	button_ok->set_option("primary", "");
	button_ok->min_width_user = 100;
	button_ok->size_mode_x = SizeMode::Shrink;
	//button_ok->enable(false);
	g3->add_child(button_ok, 5, 0);

	event("selector", [this] {
		indicator->set_color(selector->get_color());
	});
	event("ok", [this] {
		answer = selector->get_color();
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
}

base::future<color> ColorSelectionDialog::ask(Panel* parent, const string& title, const color& col, const Array<string>& params) {
	auto dlg = new ColorSelectionDialog(parent, title, col, params);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}


} // xhui
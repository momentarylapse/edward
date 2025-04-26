//
// Created by michi on 03.02.25.
//

#include "ColorSelectionDialog.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"

namespace xhui {

class ColorHSBSelector : public Control {
public:
	static constexpr float HUE_DX = 30;

	explicit ColorHSBSelector(const string& id) : Control(id) {
		col = Black;
		hue = 0;
		saturation = 0;
		brightness = 0;
	}

	color col;
	float hue, saturation, brightness;
	void set_color(const color& c) override {
		col = c;
		hue = col.hue();
		saturation = col.saturation();
		brightness = col.brightness();
		request_redraw();
	}
	color get_color() override {
		return col;
	}
	void on_left_button_down(const vec2& m) override {
		if (m.x > _area.x2 - HUE_DX) {
			hue = (m.y - _area.y1) / _area.height();
			col = color::hsb(hue, saturation, brightness, 1);
			request_redraw();
			emit_event(event_id::Changed, true);
		} else {
			saturation = (m.x - _area.x1) / (_area.width() - HUE_DX);
			brightness = 1 - (m.y - _area.y1) / _area.height();
			col = color::hsb(hue, saturation, brightness, 1);
			request_redraw();
			emit_event(event_id::Changed, true);
		}
	}

	void _draw(Painter* p) override {
		float w = _area.width();
		float h = _area.height();

		const int N = 100;
		for (int i=0; i<N; i++) {
			p->set_color(color::hsb((float)i / (float)N, 1, 1, 1));
			p->draw_rect({_area.x2 - HUE_DX, _area.x2, _area.y1 + (float)i / (float)N * h, _area.y1 + (float)(i + 1) / (float)N * h});
		}
		float ww = w - HUE_DX;
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++) {
				p->set_color(color::hsb(hue, (float)i / (float)N, 1 - (float)j / (float)N, 1));

				p->draw_rect({_area.x1 + (float)i / (float)N * ww, _area.x1 + (float)(i + 1) / (float)N * ww, _area.y1 + (float)j / (float)N * h, _area.y1 + (float)(j + 1) / (float)N * h});
			}
		}

		p->set_color(White);
		p->draw_circle(_area.p10() + vec2(- HUE_DX/2, hue * h), 5);
		p->draw_circle(_area.p00() + vec2(saturation * ww, (1 - brightness) * h), 5);
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
		p->set_color(col);
		p->set_roundness(Theme::_default.button_radius);
		p->draw_rect(_area);
		p->set_roundness(0);
	}
};

ColorSelectionDialog::ColorSelectionDialog(Panel* parent, const string& title, const color& col, const Array<string>& params) : Dialog(title, 600, 600, parent) {
	selector = new ColorHSBSelector("selector");
	selector->set_color(col);

	auto g1 = new Grid("grid1");
	add_child(g1);
	auto g2 = new Grid("grid2");
	g1->add_child(g2, 0, 0);
	g1->add_child(selector, 0, 1);

	auto g3 = new Grid("grid3");
	g1->add_child(g3, 0, 2);
	auto spacer = new Label("spacer", "");
	g3->add_child(spacer, 1, 0);
	spacer->size_mode_x = SizeMode::Expand;
	indicator = new ColorIndicator("indicator");
	indicator->set_color(col);
	indicator->min_width_user = 100;
	indicator->size_mode_x = SizeMode::Shrink;
	indicator->size_mode_y = SizeMode::Shrink;
	g3->add_child(indicator, 0, 0);
	auto button_cancel = new Button("cancel", "Cancel");
	button_cancel->min_width_user = 100;
	button_cancel->size_mode_x = SizeMode::Shrink;
	g3->add_child(button_cancel, 2, 0);
	auto button_ok = new Button("ok", "Ok");
	button_ok->min_width_user = 100;
	button_ok->size_mode_x = SizeMode::Shrink;
	//button_ok->enable(false);
	g3->add_child(button_ok, 3, 0);

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
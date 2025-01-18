#include <lib/xhui/ContextVulkan.h>

#include "lib/xhui/xhui.h"
#include "lib/xhui/controls/Button.h"
#include "lib/xhui/controls/Label.h"
#include "lib/xhui/controls/Edit.h"
#include "lib/xhui/controls/Grid.h"
#include "lib/xhui/controls/DrawingArea.h"
#include "lib/xhui/Painter.h"
#include "lib/os/msg.h"

#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

int hui_main(const Array<string>& args) {

	try {
		xhui::init();
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	auto w = new xhui::Window("test", 1024, 768);
	auto g = new xhui::Grid("grid");
	w->add(g);
	auto g2 = new xhui::Grid("grid2");
	g->add(g2, 0, 0);
	g2->add(new xhui::Label("label1", "label"), 0, 0);
	g2->add(new xhui::Button("button1", "a small test g"), 1, 0);
	g2->add(new xhui::Button("button2", "a small test g"), 2, 0);
	g->add(new xhui::DrawingArea("area"), 0, 1);

	w->event("button1", [] {
		msg_write("event button1 click");
	});
	w->event_xp("area", "hui:draw", [] (Painter* p) {
		if (true) {
			auto pp = (xhui::Painter*)p;
			auto cb = pp->cb;
			cb->clear(pp->native_area, {Red}, 0);
		} else {
			p->set_color(xhui::Theme::_default.background_low);
			p->set_roundness(8);
			p->draw_rect(p->area());
			p->set_roundness(0);
			float font_size = 50;
			p->set_font_size(font_size);
			vec2 p0 = p->area().p00() + vec2(20, 20);
			string text = "Test  g";
			auto dims = font::get_text_dimensions(text);
			p->set_color(xhui::Theme::_default.border);
			p->draw_line({p0.x, p0.y + dims.bounding_top_to_line}, {p0.x + dims.bounding_width, p0.y + dims.bounding_top_to_line});
			p->set_fill(false);
			p->draw_rect(dims.bounding_box(p0));
			p->set_fill(true);

			p->set_color(xhui::Theme::_default.text);
			p->draw_str(p0, text);

			//p->draw_str({50, 200}, "Test g\nbla gg");

			p->set_roundness(20);
			p->draw_rect({50, 300, 150, 250});
			((xhui::Painter*)p)->softness = 10;
			p->draw_rect({50, 300, 300, 400});
			((xhui::Painter*)p)->softness = 0;
		}
	});

	xhui::run();

	return 0;
}


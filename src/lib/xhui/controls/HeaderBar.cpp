#include "HeaderBar.h"
#include "Grid.h"
#include "Button.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"
#include "../../os/msg.h"

namespace xhui {

HeaderBar::HeaderBar(Window *w, const string &_id) : Control(_id) {
	expand_x = true;
	expand_y = false;
	owner = w;

	grid_left = new Grid(":header-grid-left:");
	grid_left->owner = w;
	grid_right = new Grid(":header-grid-right:");
	grid_right->owner = w;
	button_close = new Button(":header-button-close:", "Close");
	button_close->expand_x = false;
	button_close->expand_y = true;
	button_close->primary = true;
	grid_right->add(button_close, 0, 0);

	owner->event(":header-button-close:", [this] { owner->window->request_destroy(); });
}

void HeaderBar::get_content_min_size(int &w, int &h) {
	w = 0;
	h = Theme::_default.headerbar_height;
}

void HeaderBar::on_left_button_down(const vec2& m) {
	//msg_write("click header");
	dragging = true;
	drag_m0 = m;
	owner->window->get_position(window_pos_x0, window_pos_y0);
	request_redraw();
}
void HeaderBar::on_left_button_up(const vec2& m) {
	request_redraw();
	dragging = false;

	//window->handle_event(id, "hui:click");
}
void HeaderBar::on_mouse_move(const vec2& m, const vec2& d) {
	if (dragging) {
		// FIXME...
		//msg_write(str(m - drag_m0));
		owner->window->set_position(window_pos_x0 + m.x - drag_m0.x, window_pos_y0 + m.y - drag_m0.y);
	}
}

void HeaderBar::negotiate_area(const rect &available) {
	_area = available;
	int w, h;
	grid_right->get_content_min_size(w, h);
	float R = Theme::_default.spacing;
	grid_right->negotiate_area(rect(available.x2 - w - R, available.x2 - R, available.y1 + R, available.y2 - R));
}

void HeaderBar::_draw(Painter *p) {
	float R = Theme::_default.window_radius;

	// round bg
	p->set_roundness(R);
	p->set_color(Theme::_default.background_header);
	p->draw_rect(_area);

	// flat lower bg
	p->set_roundness(0);
	p->draw_rect({_area.x1, _area.x2, _area.y2 - R, _area.y2});
	p->set_color({0.2, 0,0,0});
	p->draw_line({_area.x1, _area.y2}, {_area.x2, _area.y2});

	p->set_color(Theme::_default.text);
	p->set_font_size(Theme::_default.font_size * 1.6f);
	float ww = p->get_str_width(owner->window->get_title());
	p->draw_str(_area.center() - vec2(ww/2, Theme::_default.font_size * 0.8f), owner->window->get_title());
	p->set_font_size(Theme::_default.font_size);

	grid_right->_draw(p);
}

}

#include "Panel.h"
#include "Painter.h"

namespace xhui {



Panel::Panel(const string &_id) : Control(_id) {
	ignore_hover = true;
	owner = this;

	expand_x = true;
	expand_y = true;
}

void Panel::_draw(Painter *p) {
	if (top_control)
		top_control->_draw(p);
}

void Panel::negotiate_area(const rect &available) {
	_area = available;

	/*Array<int> w, h;
	get_grid_min_sizes(w, h);
	int total_min_w, total_min_h;
	get_content_min_size(total_min_w, total_min_h);
	float diff_x = max(available.width() - total_min_w, 0.0f);
	float diff_y = max(available.height() - total_min_h, 0.0f);

	Array<float> gx, gy;
	get_grid_greed_factors(gx, gy);
	float total_greed_x, total_greed_y;
	get_greed_factor(total_greed_x, total_greed_y);

	float greed_to_x = (total_greed_x > 0) ? diff_x / total_greed_x : 0;
	float greed_to_y = (total_greed_y > 0) ? diff_y / total_greed_y : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];
	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	for (auto &c: children) {
		int x0 = _area.x1;
		int y0 = _area.y1;
		for (int i=0; i<c.x; i++)
			x0 += w[i] + spacing;
		for (int i=0; i<c.y; i++)
			y0 += h[i] + spacing;
		c.control->negotiate_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
	}*/
}

void Panel::add(Control *c) {
	top_control = c;
	c->owner = this;
	controls.add(c);
}

void Panel::event(const string &id, Callback f) {
	EventHandler e;
	e.id = id;
	e.f = f;
	event_handlers.add(e);
}

void Panel::event_x(const string &id, const string &msg, Callback f) {
	EventHandler e;
	e.id = id;
	e.msg = msg;
	e.f = f;
	event_handlers.add(e);
}

void Panel::event_xp(const string &id, const string &msg, CallbackP f) {
	EventHandler e;
	e.id = id;
	e.msg = msg;
	e.fp = f;
	event_handlers.add(e);
}

bool match_event(Panel::EventHandler& e, const string &id, const string &msg, bool is_default) {
	if (e.id != id)
		return false;
	if (e.msg == "" and is_default)
		return true;
	return e.msg == msg;
}

void Panel::handle_event(const string &id, const string &msg, bool is_default) {
	for (auto &e: event_handlers)
		if (match_event(e, id, msg, is_default) and e.f)
			e.f();
}

void Panel::handle_event_p(const string &id, const string &msg, Painter *p) {
	for (auto &e: event_handlers)
		if (match_event(e, id, msg, false) and e.fp)
			e.fp(p);
}

void Panel::set_string(const string& id, const string& text) {
	for (auto& c: controls)
		if (c->id == id)
			c->set_string(text);
}

void Panel::enable(const string& id, bool enabled) {
	for (auto& c: controls)
		if (c->id == id)
			c->enable(enabled);
}


}

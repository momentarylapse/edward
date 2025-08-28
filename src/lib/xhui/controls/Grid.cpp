#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../os/msg.h"
#include "../../base/algo.h"

namespace xhui {



int sum(const Array<int> &a) {
	int r = 0;
	for (int i=0; i<a.num; i++)
		r += a[i];
	return r;
}

float sum(const Array<float> &a) {
	float r = 0;
	for (int i=0; i<a.num; i++)
		r += a[i];
	return r;
}

Grid::Grid(const string &_id) : Control(_id) {
	ignore_hover = true;
	spacing = Theme::_default.spacing;
	margin = {0,0,0,0};

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Grid::add_child(shared<Control> c, int x, int y) {
	if (vertical)
		std::swap(x, y);
	children.add({c, x, y});
	nx = max(nx, x+1);
	ny = max(ny, y+1);
	if (owner)
		c->_register(owner);
}

void Grid::remove_child(Control* c) {
	c->_unregister();
	base::remove_if(children, [c] (const Child& child) {
		return child.control == c;
	});
}


void Grid::_draw(Painter *p) {
	if (card) {
		p->set_roundness(Theme::_default.button_radius);
		p->set_color(Theme::_default.background_raised());
		p->draw_rect(_area);
		p->set_roundness(0);
	}

	for (auto &c: children)
		if (c.control->visible)
			c.control->_draw(p);
}

void Grid::get_grid_min_sizes(Array<float> &w, Array<float> &h) const {
	w.resize(nx);
	h.resize(ny);
	for (auto &c: children) {
		if (!c.control->visible)
			continue;
		vec2 s = c.control->get_effective_min_size();
		w[c.x] = max(w[c.x], s.x);
		h[c.y] = max(h[c.y], s.y);
	}
}

vec2 Grid::get_content_min_size() const {
	Array<float> w, h;
	get_grid_min_sizes(w, h);
	vec2 s;
	s.x = sum(w) + spacing * (float)(w.num - 1) + margin.x1 + margin.x2;
	s.y = sum(h) + spacing * (float)(h.num - 1) + margin.y1 + margin.y2;
	return s;
}

vec2 Grid::get_greed_factor() const {
	Array<float> xx, yy;
	get_grid_greed_factors(xx, yy);
	vec2 f = {0, 0};
	if (size_mode_x == SizeMode::Expand)
		f.x = greed_factor.x;
	else if (size_mode_x == SizeMode::ForwardChild)
		f.x = sum(xx);
	if (size_mode_y == SizeMode::Expand)
		f.y = greed_factor.y;
	else if (size_mode_y == SizeMode::ForwardChild)
		f.y = sum(yy);
	return f;
}

void Grid::get_grid_greed_factors(Array<float> &x, Array<float> &y) const {
	x.resize(nx);
	y.resize(ny);
	for (auto& c: children)
		if (c.control->visible) {
			vec2 f = c.control->get_greed_factor();
			x[c.x] = max(x[c.x], f.x);
			y[c.y] = max(y[c.y], f.y);
		}
}

void Grid::negotiate_area(const rect &available) {
	_area = available;

	Array<float> w, h;
	get_grid_min_sizes(w, h);
	vec2 total_min_size = get_content_min_size();
	float diff_x = max(available.width() - total_min_size.x, 0.0f); //  - margin * 2 - spacing * (w.num + 1)
	float diff_y = max(available.height() - total_min_size.y, 0.0f); //  - margin * 2 - spacing * (h.num + 1)

	Array<float> gx, gy;
	get_grid_greed_factors(gx, gy);
	vec2 total_greed = get_greed_factor();

	float greed_to_x = (total_greed.x > 0) ? diff_x / total_greed.x : 0;
	float greed_to_y = (total_greed.y > 0) ? diff_y / total_greed.y : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];
	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	for (auto &c: children)
		if (c.control->visible) {
			float x0 = _area.x1 + margin.x1;
			float y0 = _area.y1 + margin.y1;
			for (int i=0; i<c.x; i++)
				x0 += w[i] + spacing;
			for (int i=0; i<c.y; i++)
				y0 += h[i] + spacing;
			c.control->negotiate_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
		}
}

Array<Control*> Grid::get_children(ChildFilter f) const {
	Array<Control*> r;
	for (auto& c: children)
		if (f == ChildFilter::All or c.control->visible)
			r.add(c.control.get());
	return r;
}

void Grid::set_option(const string& key, const string& value) {
	if (key == "margin") {
		float f = value._float();
		margin = {f,f,f,f};
	} else if (key == "spacing") {
		spacing = value._float();
	} else if (key == "class") {
		if (value == "card") {
			card = true;
			margin = {7,7,7,7};
		}
	} else if (key == "vertical") {
		vertical = true;
	} else {
		Control::set_option(key, value);
	}
	request_redraw();
}



}

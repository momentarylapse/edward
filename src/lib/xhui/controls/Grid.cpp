#include "Grid.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../../os/msg.h"

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

	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

void Grid::add(Control *c, int x, int y) {
	children.add({c, x, y});
	nx = max(nx, x+1);
	ny = max(ny, y+1);
	if (owner)
		c->_register(owner);
}

void Grid::_draw(Painter *p) {
	if (card) {
		p->set_roundness(Theme::_default.button_radius);
		p->set_color(color::interpolate(Theme::_default.background, Theme::_default.background_button, 0.5f));
		p->draw_rect(_area);
		p->set_roundness(0);
	}

	for (auto &c: children)
		if (c.control->visible)
			c.control->_draw(p);
}

void Grid::get_grid_min_sizes(Array<int> &w, Array<int> &h) {
	w.resize(nx);
	h.resize(ny);
	for (auto &c: children) {
		if (!c.control->visible)
			continue;
		int ww, hh;
		c.control->get_effective_min_size(ww, hh);
		w[c.x] = max(w[c.x], ww);
		h[c.y] = max(h[c.y], hh);
	}
}

void Grid::get_content_min_size(int &_w, int &_h) {
	Array<int> w, h;
	get_grid_min_sizes(w, h);
	_w = sum(w) + spacing * (w.num - 1) + margin * 2;
	_h = sum(h) + spacing * (h.num - 1) + margin * 2;
}

void Grid::get_greed_factor(float &_x, float &_y) {
	Array<float> xx, yy;
	get_grid_greed_factors(xx, yy);
	_x = 0;
	_y = 0;
	if (size_mode_x == SizeMode::Expand)
		_x = 1;
	else if (size_mode_x == SizeMode::ForwardChild)
		_x = sum(xx);
	if (size_mode_y == SizeMode::Expand)
		_y = 1;
	else if (size_mode_y == SizeMode::ForwardChild)
		_y = sum(yy);
}

void Grid::get_grid_greed_factors(Array<float> &x, Array<float> &y) {
	x.resize(nx);
	y.resize(ny);
	for (auto &c: children) {
		float cx, cy;
		c.control->get_greed_factor(cx, cy);
		x[c.x] = max(x[c.x], cx);
		y[c.y] = max(y[c.y], cy);
	}
}

void Grid::negotiate_area(const rect &available) {
	_area = available;

	Array<int> w, h;
	get_grid_min_sizes(w, h);
	int total_min_w, total_min_h;
	get_content_min_size(total_min_w, total_min_h);
	float diff_x = max(available.width() - total_min_w, 0.0f); //  - margin * 2 - spacing * (w.num + 1)
	float diff_y = max(available.height() - total_min_h, 0.0f); //  - margin * 2 - spacing * (h.num + 1)

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
		int x0 = _area.x1 + margin;
		int y0 = _area.y1 + margin;
		for (int i=0; i<c.x; i++)
			x0 += w[i] + spacing;
		for (int i=0; i<c.y; i++)
			y0 += h[i] + spacing;
		c.control->negotiate_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
	}
}

Array<Control*> Grid::get_children() const {
	Array<Control*> r;
	for (auto& c: children)
		r.add(c.control);
	return r;
}

void Grid::set_option(const string& key, const string& value) {
	if (key == "margin") {
		margin = value._float();
	} else if (key == "spacing") {
		spacing = value._float();
	} else if (key == "class") {
		if (value == "card") {
			card = true;
			margin = 7;
		}
	} else {
		Control::set_option(key, value);
	}
	request_redraw();
}



}

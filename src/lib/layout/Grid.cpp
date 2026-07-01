//
// Created by michi on 3/24/26.
//

#include "Grid.h"
#include <lib/base/algo.h>
#include <lib/base/iter.h>

#include "lib/os/msg.h"

namespace layout {

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

Grid::Grid(Node& _node) : node(_node) {
}

void Grid::add_child(shared<Node> c, int x, int y) {
	if (vertical)
		std::swap(x, y);
	children.add({c.get(), x, y});
	nx = max(nx, x+1);
	ny = max(ny, y+1);
}

void Grid::remove_child(Node* c) {
	base::remove_if(children, [c] (const layout::GridChild& child) {
		return child.node == c;
	});
}

void Grid::get_min_sizes(Array<float> &w, Array<float> &h) const {
	w.resize(nx);
	h.resize(ny);
	for (auto &c: children) {
		if (!c.node->visible)
			continue;
		vec2 s = c.node->get_effective_min_size();
		w[c.x] = max(w[c.x], s.x);
		h[c.y] = max(h[c.y], s.y);
	}
}

vec2 Grid::get_content_min_size() const {
	Array<float> w, h;
	get_min_sizes(w, h);
	vec2 s;
	s.x = sum(w) + spacing * (float)(w.num - 1);
	s.y = sum(h) + spacing * (float)(h.num - 1);
	return s;
}

vec2 Grid::get_greed_factor() const {
	Array<float> xx, yy;
	get_greed_factors(xx, yy);
	vec2 f = {0, 0};
	if (node.size_mode_x == SizeMode::Expand)
		f.x = node.greed_factor.x;
	else if (node.size_mode_x == SizeMode::ForwardChild)
		f.x = sum(xx);
	if (node.size_mode_y == SizeMode::Expand)
		f.y = node.greed_factor.y;
	else if (node.size_mode_y == SizeMode::ForwardChild)
		f.y = sum(yy);
	return f;
}

void Grid::get_greed_factors(Array<float> &x, Array<float> &y) const {
	x.resize(nx);
	y.resize(ny);
	for (auto& c: children)
		if (c.node->visible) {
			vec2 f = c.node->get_greed_factor();
			x[c.x] = max(x[c.x], f.x);
			y[c.y] = max(y[c.y], f.y);
		}
}

void Grid::negotiate_content_area(const rect &available) {
	Array<float> w, h;
	get_min_sizes(w, h);
	vec2 total_min_size = get_content_min_size();
	float diff_x = max(available.width() - total_min_size.x, 0.0f); //  - spacing * (w.num + 1)
	float diff_y = max(available.height() - total_min_size.y, 0.0f); //  - spacing * (h.num + 1)

	Array<float> gx, gy;
	get_greed_factors(gx, gy);
	vec2 total_greed = get_greed_factor();

	float greed_to_x = (total_greed.x > 0) ? diff_x / total_greed.x : 0;
	float greed_to_y = (total_greed.y > 0) ? diff_y / total_greed.y : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];
	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	for (auto &c: children)
		if (c.node->visible) {
			float x0 = available.x1;
			float y0 = available.y1;
			for (int i=0; i<c.x; i++)
				x0 += w[i] + spacing;
			for (int i=0; i<c.y; i++)
				y0 += h[i] + spacing;
			c.node->negotiate_outer_area(rect(x0, x0 + w[c.x], y0, y0 + h[c.y]));
		}
}

Array<Node*> Grid::get_children(ChildFilter f) const {
	Array<Node*> r;
	for (auto& c: children)
		if (f == ChildFilter::All or c.node->visible)
			r.add(c.node.get());
	return r;
}

void Grid::set_option(const string& key, const string& value) {
	if (key == "spacing") {
		spacing = value._float();
	} else if (key == "vertical") {
		vertical = true;
	}
}





vec2 hbox_get_content_min_size(const Array<Node*>& children, float spacing) {
	vec2 s = {0,0};
	for (auto c: children) {
		if (s.x > 0)
			s.x += spacing;
		if (c->visible) {
			const auto ss = c->get_effective_min_size();
			s.y = max(s.y, ss.y);
			s.x += ss.x;
		}
	}
	return s;
}

Array<float> hbox_get_greed_factors(const Array<Node*>& children) {
	Array<float> x;
	x.resize(children.num);
	for (auto&& [i, c]: enumerate(children))
		if (c->visible) {
			if (c->size_mode_x == SizeMode::Fill)
				x[i] = 1;
			else if (c->size_mode_x == SizeMode::Expand)
				x[i] = c->get_greed_factor().x;
		}
	return x;
}

vec2 hbox_get_greed_factor(const Node* self, const Array<Node*>& children) {
	Array<float> xx = hbox_get_greed_factors(children);
	vec2 f = {0, 0};
	if (self->size_mode_x == SizeMode::Expand)
		f.x = self->greed_factor.x;
	else if (self->size_mode_x == SizeMode::ForwardChild)
		f.x = sum(xx);
	return f;
}

Array<float> hbox_get_min_widths(const Array<Node*>& children) {
	Array<float> w;
	w.resize(children.num);
	for (auto&& [i, c]: enumerate(children))
		if (c->visible)
			w[i] = c->get_effective_min_size().x;
	return w;
}

void hbox_negotiate_content_area(const Node* self, const rect& available, const Array<Node*>& children, float spacing) {
	auto w = hbox_get_min_widths(children);
	vec2 total_min_size = hbox_get_content_min_size(children, spacing);
	float diff_x = max(available.width() - total_min_size.x, 0.0f); //  - spacing * (w.num + 1)

	auto gx = hbox_get_greed_factors(children);
	float total_greed_x = sum(gx);

	float greed_to_x = (total_greed_x > 0) ? diff_x / total_greed_x : 0;

	for (int i=0; i<w.num; i++)
		w[i] += greed_to_x * gx[i];

	float x0 = available.x1;
	for (auto&& [i, c]: enumerate(children)) {
		if (c->visible) {
			float x1 = x0 + w[i];
			if (i == children.num - 1)
				x1 = available.x2;
			c->negotiate_outer_area(rect(x0, x1, available.y1, available.y2));
			x0 = x1;
		}
		x0 += spacing;
	}
}




vec2 vbox_get_content_min_size(const Array<Node*>& children, float spacing) {
	vec2 s = {0,0};
	for (auto c: children) {
		if (s.y > 0)
			s.y += spacing;
		if (c->visible) {
			const auto ss = c->get_effective_min_size();
			s.x = max(s.x, ss.x);
			s.y += ss.y;
		}
	}
	return s;
}

Array<float> vbox_get_greed_factors(const Array<Node*>& children) {
	Array<float> y;
	y.resize(children.num);
	for (auto&& [i, c]: enumerate(children))
		if (c->visible) {
			if (c->size_mode_y == SizeMode::Fill)
				y[i] = 1;
			else if (c->size_mode_y == SizeMode::Expand)
				y[i] = c->get_greed_factor().y;
		}
	return y;
}

vec2 vbox_get_greed_factor(const Node* self, const Array<Node*>& children) {
	Array<float> yy = vbox_get_greed_factors(children);
	vec2 f = {0, 0};
	if (self->size_mode_y == SizeMode::Expand)
		f.y = self->greed_factor.y;
	else if (self->size_mode_y == SizeMode::ForwardChild)
		f.y = sum(yy);
	return f;
}

Array<float> vbox_get_min_heights(const Array<Node*>& children) {
	Array<float> h;
	h.resize(children.num);
	for (auto&& [i, c]: enumerate(children))
		if (c->visible)
			h[i] = c->get_effective_min_size().y;
	return h;
}

void vbox_negotiate_content_area(const Node* self, const rect& available, const Array<Node*>& children, float spacing) {
	auto h = vbox_get_min_heights(children);
	vec2 total_min_size = vbox_get_content_min_size(children, spacing);
	float diff_y = max(available.height() - total_min_size.y, 0.0f); //  - spacing * (h.num + 1)

	auto gy = vbox_get_greed_factors(children);
	float total_greed_y = sum(gy);

	float greed_to_y = (total_greed_y > 0) ? diff_y / total_greed_y : 0;

	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];

	float y0 = available.y1;
	for (auto&& [i, c]: enumerate(children)) {
		if (c->visible) {
			float y1 = y0 + h[i];
			if (i == children.num - 1)
				y1 = available.y2;
			c->negotiate_outer_area(rect(available.x1, available.x2, y0, y1));
			y0 = y1;
		}
		y0 += spacing;
	}
}
} // layout
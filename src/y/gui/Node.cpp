/*
 * Layer.cpp
 *
 *  Created on: Aug 10, 2020
 *      Author: michi
 */

#include "Node.h"
#include "gui.h"
#include <EngineData.h>
#include <lib/layout/Resource.h>
#include <lib/layout/Node.h>
#include <lib/ygraphics/graphics-impl.h>
#include "Text.h"
#include "lib/base/iter.h"
#include "lib/os/msg.h"

namespace gui {

using SizeMode = layout::SizeMode;

Node::Node() : layout::Node(p2s(this)) {
	type = Type::NODE;
	dz = 1;
	col = White;
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	align = {0, 0};

	eff_col = White;
	eff_z = 0;

	allow_hover = false;

	parent = nullptr;
}

Node::~Node() = default;

void Node::add(shared<Node> n) {
	children.add(n);
	n->parent = this;
	update_tree();
}

void Node::remove(Node &n) {
	for (int i=0; i<children.num; i++)
		if (children[i] == &n)
			children.erase(i);
	update_tree();
}

void Node::remove_all_children() {
	children.clear();
	update_tree();
}

void Node::set_area(const rect &r) {
	margin.x1 = r.x1;
	margin.y1 = r.y1;
	min_width_user = r.width();
	min_height_user = r.height();
	align = {0,0};
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

Node* Node::get(const string& _id) {
	if (id == _id)
		return this;
	for (auto n: weak(children)) {
		if (n->id == _id)
			return n;
		if (auto c = n->get(_id))
			return c;
	}
	return nullptr;
}

void print_resource(const layout::Resource& r, const string& prefix) {
	msg_write(format("%s%s %s  %s", prefix, r.type, r.id, str(r.options)));
	for (const auto& c: r.children)
		print_resource(c, prefix + "    ");
}

void Node::apply_resource(const layout::Resource &r) {
	if (r.id != "?" and r.id != "")
		id = r.id;
	//n->align = Align::NONE;
	for (const auto& o: r.options)
		set_option(o.key, o.value);

	for (const auto& c: r.children) {
		if (Node* n = create_node(c.type)) {
			//msg_write("create " + c.type + "   " + p2s(n));
			add(n);
			n->apply_resource(c);
		}
	}
}


void Node::add_from_source(const string& source) {
	auto r = layout::Resource::parse(source, false);
	print_resource(r, "");
	//if (r.id != "?")

	apply_resource(r);
}

Array<const layout::Node*> Node::_get_children(layout::ChildFilter f) const {
	Array<const layout::Node*> r;
	for (auto c: weak(children))
		if (c->visible)
			r.add(c);
	return r;
}

void Node::negotiate_content_area(const rect &available) {
	for (auto c: weak(children))
		c->negotiate_outer_area(available);
}

void Node::set_option(const string& key, const string& value) {
	if (key == "x") {
		margin.x1 = value._float();
		align.x = 0;
		size_mode_x = SizeMode::Shrink;
	} else if (key == "y") {
		margin.y1 = value._float();
		align.y = 0;
		size_mode_y = SizeMode::Shrink;
	} else if (key == "dz") {
		dz = value._float();
	} else if (key == "color") {
		col = color::parse(value);
	} else if (key == "allowhover") {
		allow_hover = value == "" or value._bool();
	} else if (key == "ignorehover") {
		allow_hover = false;
	} else if (key == "nonsquare") {
		//non_square = true;
		msg_error("TOTO Node.nonsquare");
	} else {
		layout::Node::set_option(key, value);
	}
}


HBox::HBox() {
	type = Type::HBOX;
	size_mode_x = SizeMode::Fill;
	size_mode_y = SizeMode::Fill;
}



VBox::VBox() {
	type = Type::VBOX;
	size_mode_x = SizeMode::Fill;
	size_mode_y = SizeMode::Fill;
}

vec2 VBox::get_content_min_size() const {
	vec2 s = {0,0};
	for (auto c: weak(children)) {
		if (s.y > 0)
			s.y += spacing;
		if (c->visible) {
			const auto ss = c->get_content_min_size();
			s.x = max(s.x, ss.x);
			s.y += ss.y;
		}
	}
	return s;
}

vec2 VBox::get_greed_factor() const {
	return Node::get_greed_factor();
}

Array<float> VBox::get_min_heights() const {
	Array<float> h;
	for (auto c: weak(children)) {
		if (c->visible) {
			const vec2 s = c->get_effective_min_size();
			h.add(s.y);
		} else {
			h.add(0);
		}
	}
	return h;
}

void VBox::negotiate_content_area(const rect& available) {
	auto h = get_min_heights();
	vec2 total_min_size = get_content_min_size();
	float diff_x = max(available.width() - total_min_size.x, 0.0f); //  - spacing * (w.num + 1)
	float diff_y = max(available.height() - total_min_size.y, 0.0f); //  - spacing * (h.num + 1)

	/*vec2 total_greed = get_greed_factor();

	float greed_to_y = (total_greed.y > 0) ? diff_y / total_greed.y : 0;

	for (int i=0; i<h.num; i++)
		h[i] += greed_to_y * gy[i];*/

	float y0 = available.y1;
	for (auto&& [i, c]: enumerate(weak(children))) {
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

void VBox::set_option(const string &key, const string &value) {
	if (key == "spacing") {
		spacing = value._float();
	} else {
		Node::set_option(key, value);
	}
}
}

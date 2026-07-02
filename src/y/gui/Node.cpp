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
#include <lib/layout/Grid.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>

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

void Node::set_pos(const vec2& pos) {
	margin.x1 = pos.x;
	margin.y1 = pos.y;
	align = {0,0};
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

void Node::set_area(const rect &r) {
	set_pos(r.p00());
	min_width_user = r.width();
	min_height_user = r.height();
}

Node* Node::get(const string& _id) {
	if (id == _id)
		return this;
	for (auto n: weak(children))
		if (auto c = n->get(_id))
			return c;
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

	for (const auto& c: r.children)
		add_from_resource(c);
}

Node* Node::add_from_resource(const layout::Resource& r) {
	if (Node* n = create_node(r.type)) {
		//msg_write("create... " + c.type + "   " + p2s(n));
		add(n);
		n->apply_resource(r);
		return n;
	}
	return nullptr;
}

Node* Node::add_from_source(const string& source) {
	auto r = layout::Resource::parse(source, false);
	print_resource(r, "");
	//if (r.id != "?")

	return add_from_resource(r);
}

Array<const layout::Node*> Node::_get_children(layout::ChildFilter f) const {
	Array<const layout::Node*> r;
	for (auto c: weak(children))
		if (c->visible or f == layout::ChildFilter::All)
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
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

vec2 HBox::get_content_min_size() const {
	return layout::hbox_get_content_min_size(weak_nodes(children), spacing);
}

void HBox::negotiate_content_area(const rect& available) {
	hbox_negotiate_content_area(this, available, weak_nodes(children), spacing);
}

void HBox::set_option(const string &key, const string &value) {
	if (key == "spacing") {
		spacing = value._float();
	} else {
		Node::set_option(key, value);
	}
}



VBox::VBox() {
	type = Type::VBOX;
	size_mode_x = SizeMode::ForwardChild;
	size_mode_y = SizeMode::ForwardChild;
}

vec2 VBox::get_content_min_size() const {
	return layout::vbox_get_content_min_size(weak_nodes(children), spacing);
}

void VBox::negotiate_content_area(const rect& available) {
	vbox_negotiate_content_area(this, available, weak_nodes(children), spacing);
}

void VBox::set_option(const string &key, const string &value) {
	if (key == "spacing") {
		spacing = value._float();
	} else {
		Node::set_option(key, value);
	}
}
}

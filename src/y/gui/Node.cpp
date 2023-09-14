/*
 * Layer.cpp
 *
 *  Created on: Aug 10, 2020
 *      Author: michi
 */

#include "Node.h"
#include "gui.h"
#include "../y/EngineData.h"
//#include <algorithm>

namespace gui {

Node::Node(const rect &r) {
	type = Type::NODE;
	set_area(r);
	dz = 1;
	col = White;
	visible = true;
	margin = rect::EMPTY;
	align = Align::_TOP_LEFT;

	eff_col = White;
	eff_area = r;
	eff_z = 0;
	eff_visible = true;

	parent = nullptr;
}

Node::~Node() {
}

void Node::__init_base__(const rect &r) {
	new(this) Node(r);
}

void Node::__delete__() {
	this->Node::~Node();
}

void Node::add(shared<Node> n) {
	children.add(n);
	n->parent = this;
	update_tree();
}

rect rect_move_x(const rect &r, float dx) {
	return rect(r.x1 + dx, r.x2 + dx, r.y1, r.y2);
}

rect rect_move_y(const rect &r, float dy) {
	return rect(r.x1, r.x2, r.y1 + dy, r.y2 + dy);
}

rect rect_sub_margin(const rect &r, const rect &m) {
	return rect(r.x1 + m.x1, r.x2 - m.x2, r.y1 + m.y1, r.y2 - m.y2);
}

void Node::set_area(const rect &r) {
	pos.x = r.x1;
	pos.y = r.y1;
	width = r.width();
	height = r.height();
}

void Node::update_geometry(const rect &target) {
	if (parent) {
		eff_z = parent->eff_z + dz;
		eff_col = col;//parent->eff_col * col;
		eff_visible = parent->eff_visible and visible;
	} else {
		// toplevel
		eff_col = col;
		eff_z = 0;
		eff_visible = visible;
	}

	if (parent) {
		eff_area = target;
		float fx = 1/engine.physical_aspect_ratio;
		if (align & Align::NONSQUARE)
			fx = 1;

		if (align & Align::FILL_X) {
			eff_area.x1 = target.x1 + margin.x1 * fx;
			eff_area.x2 = target.x2 - margin.x2 * fx;
		} else if (align & Align::LEFT) {
			eff_area.x1 = target.x1 + (margin.x1 + pos.x) * fx;
			eff_area.x2 = target.x1 + (margin.x1 + pos.x + width) * fx;
		} else if (align & Align::CENTER_H) {
			eff_area.x1 = target.center().x + (pos.x - width / 2) * fx;
			eff_area.x2 = target.center().x + (pos.x + width / 2) * fx;
		} else if (align & Align::RIGHT) {
			eff_area.x1 = target.x2 + (pos.x - margin.x2 - width) * fx;
			eff_area.x2 = target.x2 + (pos.x - margin.x2) * fx;
		}

		if (align & Align::FILL_Y) {
			eff_area.y1 = target.y1 + margin.y1;
			eff_area.y2 = target.y2 - margin.y2;
		} else if (align & Align::TOP) {
			eff_area.y1 = target.y1 + margin.y1 + pos.y;
			eff_area.y2 = target.y1 + margin.y1 + pos.y + height;
		} else if (align & Align::CENTER_V) {
			eff_area.y1 = target.center().y + pos.y - height / 2;
			eff_area.y2 = target.center().y + pos.y + height / 2;
		} else if (align & Align::BOTTOM) {
			eff_area.y1 = target.y2 + (pos.y - margin.y2 - height);
			eff_area.y2 = target.y2 + (pos.y - margin.y2);
		}

		//eff_area = rect_sub_margin(eff_area, margin);

	} else {
		// toplevel
		eff_area = target;
	}

	auto sub_area = eff_area;
	for (auto n: children) {
		n->update_geometry(sub_area);
		if (type == Type::VBOX)
			sub_area.y1 = n->eff_area.y2 + n->margin.y2;
		if (type == Type::HBOX)
			sub_area.x1 = n->eff_area.x2 + n->margin.x2;
	}
}


HBox::HBox() : Node(rect::ID) {
	type = Type::HBOX;
	align = Align::_FILL_XY;
}

void HBox::__init__() {
	new(this) HBox();
}



VBox::VBox() : Node(rect::ID) {
	type = Type::VBOX;
	align = Align::_FILL_XY;
}

void VBox::__init__() {
	new(this) VBox();
}

}

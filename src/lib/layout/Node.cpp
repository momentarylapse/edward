#include "Node.h"
#include <lib/base/algo.h>
#include <lib/os/msg.h>

namespace layout {



Node::Node(const string &_id) {
	id = _id;
	min_width_user = -1;
	min_height_user = -1;
	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Expand;
	area = rect::EMPTY;
}

Node::~Node() = default;

/*Array<Control*> Node::get_children_recursive(bool include_me, ChildFilter f) const {
	Array<Control*> r;
	if (include_me)
		r.add(const_cast<Control*>(this));
	for (auto c: get_children(f))
		r.append(c->get_children_recursive(true, f));
	return r;
}*/



vec2 Node::get_content_min_size() const {
	return {0,0};
}

vec2 Node::get_greed_factor() const {
	vec2 f = greed_factor;
	if (size_mode_x != SizeMode::Expand)
		f.x = 0;
	if (size_mode_y != SizeMode::Expand)
		f.y = 0;
	return f;
}

vec2 Node::get_effective_min_size() const {
	vec2 s = get_content_min_size();
	if (min_width_user >= 0)
		s.x = min_width_user;
	if (min_height_user >= 0)
		s.y = min_height_user;
	return s;
}

void Node::negotiate_area(const rect &available) {
	area = available;
	if (size_mode_x != SizeMode::Expand or size_mode_y != SizeMode::Expand) {
		const auto min_size = get_effective_min_size();
		//if (size_mode_x == SizeMode::Expand)
		if (size_mode_x == SizeMode::Shrink) {
			area.x1 = available.center().x - min_size.x / 2;
			area.x2 = available.center().x + min_size.x / 2;
		}
		if (size_mode_y == SizeMode::Shrink) {
			area.y1 = available.center().y - min_size.y / 2;
			area.y2 = available.center().y + min_size.y / 2;
		}
	}
}

void Node::set_layout_option(const string& key, const string& value) {
	if (key == "expandx") {
		size_mode_x = SizeMode::Expand;
		if (value._bool())
			size_mode_x = SizeMode::Fill;
	} else if (key == "expandy") {
		size_mode_y = SizeMode::Expand;
		if (value._bool())
			size_mode_y = SizeMode::Fill;
	} else if (key == "noexpandx" or key == "fillx") {
		size_mode_x = SizeMode::Fill;
	} else if (key == "noexpandy" or key == "filly") {
		size_mode_y = SizeMode::Fill;
	} else if (key == "shrinkx") {
		size_mode_x = SizeMode::Shrink;
	} else if (key == "shrinky") {
		size_mode_y = SizeMode::Shrink;
	} else if (key == "width") {
		min_width_user = value._float();
		size_mode_x = SizeMode::Shrink;
	} else if (key == "height") {
		min_height_user = value._float();
		size_mode_y = SizeMode::Shrink;
	} else if (key == "greedfactorx") {
		greed_factor.x = value._float();
		size_mode_x = SizeMode::Expand;
	} else if (key == "greedfactory") {
		greed_factor.y = value._float();
		size_mode_y = SizeMode::Expand;
	} else if (key == "hidden") {
		visible = false;
	} else if (key == "visible") {
		visible = value._bool() or value == "";
	}
}


}

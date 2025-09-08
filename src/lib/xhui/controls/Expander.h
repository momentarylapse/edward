//
// Created by Michael Ankele on 2025-04-16.
//

#pragma once

#include "Control.h"
#include "Label.h"
#include "../Animator.h"

namespace xhui {

class Expander : public Control {
public:
	Expander(const string& id, const string& title);
	void negotiate_area(const rect& available) override;
	vec2 get_content_min_size() const override;
	vec2 get_greed_factor() const override;
	void _draw(Painter* p) override;

	void set_string(const string& s) override;
	void expand(bool expanded) override;
	void set_option(const string &key, const string &value) override;

	bool show_header = false;
	enum class State {
		Undecided, // "compact" but never rendered... so we can expand/compactify without animation
		Compact,
		Expanded,
		Expanding,
		Shrinking
	} state;
	Label header;
	shared<Control> child;
	Array<Control*> get_children(ChildFilter f) const override;
	void add_child(shared<Control> c, int x, int y) override;
	void remove_child(Control* c) override;

	Animator animator{this};
};

} // xhui

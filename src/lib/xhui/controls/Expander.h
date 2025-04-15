//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef EXPANDER_H
#define EXPANDER_H

#include "Control.h"
#include "Label.h"

namespace xhui {

class Expander : public Control {
public:
	Expander(const string& id, const string& title);
	void negotiate_area(const rect& available) override;
	vec2 get_content_min_size() const override;
	vec2 get_greed_factor() const override;
	void _draw(Painter* p) override;

	void set_string(const string& s) override;

	bool expanded = true;
	Label header;
	shared<Control> child;
	Array<Control*> get_children(ChildFilter f) const override;
	void add_child(shared<Control> c, int x, int y) override;
	void remove_child(Control* c) override;
};

} // xhui

#endif //EXPANDER_H

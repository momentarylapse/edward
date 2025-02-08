//
// Created by Michael Ankele on 2025-02-02.
//

#ifndef GROUP_H
#define GROUP_H

#include "Control.h"

namespace xhui {

class Group : public Control {
public:
	Group(const string& id, const string& title);
	void negotiate_area(const rect& available) override;
	void get_content_min_size(int& w, int& h) override;
	void get_greed_factor(float& x, float& y) override;
	void _draw(Painter* p) override;

	string title;
	void set_string(const string& s) override;

	shared<Control> child;
	Array<Control*> get_children() const override;
	void add(Control* c, int x, int y) override;
	void remove_child(Control* c) override;
};

} // xhui

#endif //GROUP_H

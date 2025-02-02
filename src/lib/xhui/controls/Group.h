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
	void _draw(Painter* p) override;

	string title;
	void set_string(const string& s) override;

	Control* child = nullptr;
	Array<Control*> get_children() const override;
	void add(Control* c, int x, int y) override;
};

} // xhui

#endif //GROUP_H

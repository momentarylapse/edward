//
// Created by michi on 09.02.25.
//

#ifndef TABCONTROL_H
#define TABCONTROL_H

#include "Control.h"

namespace xhui {

class TabControlHeader;

class TabControl : public Control {
public:
	TabControl(const string& id, const string& title);

	void negotiate_area(const rect& available) override;
	vec2 get_content_min_size() const override;
	vec2 get_greed_factor() const override;
	void add_child(shared<Control> c, int x, int y) override;
	Array<Control*> get_children(ChildFilter f) const override;
	void _draw(Painter* p) override;
	void set_option(const string& key, const string& value) override;

	void set_int(int i) override;
	int get_int() override;

	bool show_header = true;
	owned<TabControlHeader> header;
	struct Page {
		string header;
		shared<Control> child;
	};
	Array<Page> pages;
	int current_page;
};

} // xhui

#endif //TABCONTROL_H

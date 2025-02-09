#pragma once

#include "Control.h"

namespace xhui {



class Label : public Control {
public:
	Label(const string &id, const string &title);

	void set_string(const string &s) override;

	void _draw(Painter *p) override;
	void get_content_min_size(int &w, int &h) const override;

	string title;
	State state;

	mutable int text_w, text_h;
};

}
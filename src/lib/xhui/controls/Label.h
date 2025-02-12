#pragma once

#include "Control.h"

namespace xhui {

struct XImage;

class Label : public Control {
public:
	Label(const string &id, const string &title);

	void set_string(const string &s) override;
	void set_option(const string& key, const string& value) override;

	void _draw(Painter *p) override;
	void get_content_min_size(int &w, int &h) const override;

	string title;
	enum class Align {
		Left,
		Center,
		Right
	} align;
	float margin_x;

	XImage* image = nullptr;

	mutable int text_w, text_h;
};

}
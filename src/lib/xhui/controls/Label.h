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
	vec2 get_content_min_size() const override;

	mutable float ui_scale = 1.0f;
	float font_size;
	bool bold;
	bool italic;
	string title;
	enum class Align {
		Left,
		Center,
		Right
	} align;
	rect margin;

	XImage* image = nullptr;

	mutable float text_w, text_h;
};

}
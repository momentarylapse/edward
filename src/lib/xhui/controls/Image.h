#pragma once

#include "Control.h"

namespace xhui {

struct XImage;

class Image : public Control {
public:
	Image(const string &id, const string &title);

	void set_string(const string& s) override;
	void set_option(const string& key, const string& value) override;

	void _draw(Painter *p) override;
	vec2 get_content_min_size() const override;

	rect margin;

	XImage* image = nullptr;
};

}
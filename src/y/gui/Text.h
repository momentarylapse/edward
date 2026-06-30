/*
 * Text.h
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#pragma once

#include "Picture.h"

namespace ygfx {
	struct Face;
}

namespace gui {

class Text : public Picture {
public:
	Text();
	Text(const string &t, float h, const vec2 &p);
	~Text() override;

	vec2 get_content_min_size() const override;
	void set_option(const string &k, const string &v) override;

	string text;
	float font_size;
	ygfx::Face* font;
};

}

/*
 * Text.h
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#pragma once

#include "Picture.h"

namespace gui {

class Font;

class Text : public Picture {
public:
	Text();
	Text(const string &t, float h, const vec2 &p);
	~Text() override;

	void rebuild();
	void set_text(const string &t);

	void _set_option(const string &k, const string &v);

	string text;
	float font_size;
	Font *font;
};

}

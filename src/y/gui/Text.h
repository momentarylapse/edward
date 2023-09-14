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
	Text(const string &t, float h, const vec2 &p);
	~Text() override;
	void __init__(const string &t, float h, const vec2 &p);
	void __delete__() override;

	void rebuild();
	void set_text(const string &t);

	string text;
	float font_size;
	Font *font;
};

}

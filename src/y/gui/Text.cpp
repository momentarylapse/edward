/*
 * Text.cpp
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#include "Text.h"
#include "Font.h"
#include <lib/math/vec2.h>
#include <lib/image/image.h>
#include <lib/layout/Node.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/TextCache.h>

namespace gui {

extern ygfx::TextCache* text_cache;
extern float ui_scale;


Text::Text() : Text("", 0.05f, {0,0}) {}

Text::Text(const string &t, float h, const vec2 &p) : Picture(rect(p.x,p.x,p.y,p.y), nullptr) {//rect::ID
	type = Type::TEXT;
	//margin = rect(x, h/6, y, h/10);
	margin.x1 = p.x;
	margin.y1 = p.y;
	min_width_user = -1;
	min_height_user = -1;
	//font = Font::_default;
	size_mode_x = layout::SizeMode::Shrink;
	size_mode_y = layout::SizeMode::Shrink;
	font_size = h;
	if (font_manager)
		font = font_manager->default_font_regular;
	text = t;
	allow_hover = true;
}

Text::~Text() = default;

vec2 Text::get_content_min_size() const {
	auto& tc = text_cache->get_dimensions(text, font, font_size, ui_scale);
	return tc.inner_box({0,0}).size() / ui_scale;
}

void Text::set_option(const string &k, const string &v) {
	if (k == "size") {
		font_size = v._float();
	} else if (k == "text") {
		text = v;
	} else {
		Picture::set_option(k, v);
	}
}


}

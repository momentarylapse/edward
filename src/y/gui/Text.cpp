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
#include <lib/ygraphics/graphics-impl.h>
#include <y/EngineData.h>


namespace gui {


//Text::Text(const string &t, float h, const vec2 &p) : Picture(rect(p.x,p.x,p.y,p.y), nullptr) {//rect::ID
Text::Text() : Text(":::fake:::", 0.05f, {0,0}) {}

Text::Text(const string &t, float h, const vec2 &p) : Picture(rect(p.x,p.x,p.y,p.y), nullptr) {//rect::ID
	type = Type::TEXT;
	//margin = rect(x, h/6, y, h/10);
	font = Font::_default;
	font_size = h;
//	texture = nullptr; // needed, so we don't mess with tex_white...
	if (t != ":::fake:::")
		set_text(t);
}

Text::~Text() = default;

void Text::rebuild() {
	if (!font)
		return;
	Image im;
	font->render_text(text, align, im);
	if (im.width == 0 or im.height == 0)
		im.create(1,1, color(0,0,0,0));

	if (texture == nullptr)
		texture = new ygfx::Texture();

	texture->write(im);
	//texture->set_options("magfilter=nearest,wrap=clamp");
	texture->set_options("magfilter=linear,wrap=clamp");

	height = font_size * font->get_height_rel(text);
	width = height * (float)im.width / (float)im.height;
	if (align & Align::NONSQUARE)
		 width /= engine.physical_aspect_ratio;
}

void Text::set_text(const string &t) {
	if (t != text or !texture) {
		text = t;
		rebuild();
	}
}

void Text::_set_option(const string &k, const string &v) {
	if (k == "size") {
		font_size = v._float();
		rebuild();
	} else if (k == "text") {
		set_text(v);
	} else {
		Picture::_set_option(k, v);
	}
}


}

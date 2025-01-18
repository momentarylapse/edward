#pragma once

#include "../../base/base.h"

class Image;
class rect;
class vec2;

namespace xhui {
enum class Align;
}

namespace font {

void init();

struct TextDimensions {
	float bounding_width;
	float bounding_height;
	float bounding_top_to_line;
	float line_dy;
	float dx;
	int num_lines;

	float inner_height() const;
	rect bounding_box(const vec2& p0) const;
	rect inner_box(const vec2& p0) const;
};

void set_font(const string &font_name, float font_size);
float get_text_width(const string &text);
TextDimensions get_text_dimensions(const string &text);
void render_text(const string &text, xhui::Align align, Image &im);

}

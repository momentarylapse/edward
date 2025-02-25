#pragma once

#include "../../base/base.h"

class Image;
class rect;
class vec2;

namespace xhui {
enum class Align;
}


typedef struct FT_FaceRec_*  FT_Face;

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

struct Face {
	string name;
	bool bold, italic;
	FT_Face face;
	float current_size = 0;

	void set_size(float size);
	float units_to_pixel(float units) const;
	float get_text_width(const string &text);
	TextDimensions get_text_dimensions(const string &text);
	void render_text(const string &text, xhui::Align align, Image &im);
};

Face* load_face(const string& name, bool bold, bool italic);

/*void set_font(const string &font_name, float font_size);
float get_text_width(const string &text);
TextDimensions get_text_dimensions(const string &text);
void render_text(const string &text, xhui::Align align, Image &im);*/

}

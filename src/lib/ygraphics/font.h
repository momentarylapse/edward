#pragma once

#include <lib/base/base.h>

class Image;
struct rect;
struct vec2;


typedef struct FT_FaceRec_*  FT_Face;

namespace font {


enum class Align {
	RIGHT = 1,
	CENTER_H = 2,
	LEFT = 4
};

void init();

struct TextDimensions {
	float bounding_width;
	float bounding_height; // = num_lines * line_dy
	float line_dy;
	float ascender;
	float descender; // positive
	int num_lines;

	float inner_height() const;
	rect bounding_box(const vec2& p0) const; // with padding below last line
	rect inner_box(const vec2& p0) const; // without padding
};

struct Face {
	string name;
	bool bold, italic;
	FT_Face face;
	float current_size = 0;

	int tab_dx = 0;

	void set_size(float size);
	float units_to_pixel(float units) const;
	float get_text_width(const string &text) const;
	TextDimensions get_text_dimensions(const string &text) const;
	void render_text(const string &text, Align align, Image &im);
};

Face* load_face(const string& name, bool bold, bool italic);

/*void set_font(const string &font_name, float font_size);
float get_text_width(const string &text);
TextDimensions get_text_dimensions(const string &text);
void render_text(const string &text, xhui::Align align, Image &im);*/

}

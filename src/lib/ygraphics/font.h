#pragma once

#include <lib/base/base.h>

class Path;
class Image;
struct rect;
struct vec2;


typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_LibraryRec_* FT_Library;

namespace ygfx {


enum class Align {
	RIGHT = 1,
	CENTER_H = 2,
	LEFT = 4
};

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
	float get_text_width(const string& text) const;
	TextDimensions get_text_dimensions(const string& text) const;
	void render_text(const string& text, Align align, Image& im);
};

class FontManager {
public:
	FontManager();

	void try_load_defaults(const Array<string>& font_names, const Array<string>& font_names_mono);

	// might return null:
	Face* load(const string& name, bool bold, bool italic);
	Face* load_first(const Array<string>& name, bool bold, bool italic);

	Array<Path> directories;
	Face* default_font_regular = nullptr;
	Face* default_font_bold = nullptr;
	Face* default_font_mono_regular = nullptr;
	Face* default_font_mono_bold = nullptr;
	Face* pick(const string &font, bool bold, bool italic) const;

	FT_Library ft2;
	Array<Face*> faces;
};

}

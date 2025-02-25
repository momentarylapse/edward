#include "font.h"
#include "../../image/image.h"
#include "../../math/rect.h"
#include "../../math/vec2.h"
#include "../../os/msg.h"
#include "../../os/path.h"
#include "../../os/filesystem.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace font {

static FT_Library ft2;
static float dpi = 96;
//static FT_Face face;
//static float current_font_size = 0;
//static string current_font_name;

/*struct Face {
	FT_Face face;
};*/


void init() {
	auto error = FT_Init_FreeType(&ft2);
	if (error) {
		throw Exception("can not initialize freetype2 library");
	}
}

Array<Face*> faces;

Face* load_face(const string& name, bool bold, bool italic) {
	Face* face = new Face;
	face->name = name;
	face->bold = bold;
	face->italic = italic;

	string type = "Regular";
	if (bold)
		type = "Bold";

	auto try_load_font = [face, &type] (const Path& filename) {
		if (!os::fs::exists(filename))
			return false;
		int error = FT_New_Face(ft2, filename.c_str(), -1, &face->face);
		if (error == FT_Err_Unknown_File_Format) {
			msg_error("font unsupported: " + str(filename));
			return false;
		} else if (error) {
			msg_error("font can not be loaded: " + str(filename));
			return false;
		}
		for (int i=0; i<face->face->num_faces; i++) {
			error = FT_New_Face(ft2, filename.c_str(), i, &face->face);
			if (error)
				continue;
			if (string(face->face->style_name) == type)
				break;
		}

		return true;
	};

	//msg_write(os::fs::current_directory().str());
	if (!try_load_font(format("/System/Library/Fonts/%s.ttc", name)))
	if (!try_load_font(format("/usr/share/fonts/noto/%s-%s.ttf", name, type)))
	if (!try_load_font(format("/usr/share/fonts/open-sans/%s-%s.ttf", name, type)))
	if (!try_load_font(format("static/%s-%s.ttf", name, type))) {
		delete face;
		return nullptr;
	}


	faces.add(face);
	return face;
}

void Face::set_size(float size) {
	if (size == current_size)
		return;
	// size: points<<6
	FT_Set_Char_Size(face, 0, int(size*64.0f), (int)dpi, (int)dpi);
	current_size = size;
}

float Face::units_to_pixel(float units) const {
	// 72 pt/inch
	return units / (float)face->units_per_EM * current_size * dpi / 72.0f;
}

TextDimensions Face::get_text_dimensions(const string &text) {
	auto utf32 = text.utf8_to_utf32();
	TextDimensions dim;

	//auto glyph_index = FT_Get_Char_Index(face, 'A');
	//msg_write(glyph_index);
	//errpr = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); //load_flags);
	//error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); //render_mode);

	int wmax = 0;
	int x = 0;
	dim.num_lines = 1;

	//msg_write(face->height);
	//msg_write(face->units_per_EM);

	for (int u: utf32) {
		if (u == '\n') {
			wmax = max(wmax, x);
			x = 0;
			dim.num_lines ++;
			continue;
		}
		int error = FT_Load_Char(face, u, FT_LOAD_DEFAULT);
		if (error) {
			msg_write("E");
			continue;
		}
		//wmax = max(wmax, x + face->glyph->width);
		x += face->glyph->advance.x >> 6;
	}
	dim.dx = max(x, wmax);
	dim.bounding_width = max(x, wmax);// + current_font_size*0.1f;
	//dim.line_dy = current_font_size - units_to_pixel((float)face->descender);
	dim.line_dy = units_to_pixel((float)face->height);
	dim.bounding_top_to_line = current_size;
	dim.bounding_height = dim.line_dy * (float)dim.num_lines;
	//msg_write(f2s(units_to_pixel((float)face->descender), 3));
	return dim;
}

float TextDimensions::inner_height() const {
	return bounding_height - line_dy + bounding_top_to_line;
}

rect TextDimensions::bounding_box(const vec2& p0) const {
	return rect(p0.x, p0.x + bounding_width, p0.y, p0.y + bounding_height);
}
rect TextDimensions::inner_box(const vec2& p0) const {
	return rect(p0.x, p0.x + bounding_width, p0.y, p0.y + inner_height());
}


float Face::get_text_width(const string &text) {
	auto dim = get_text_dimensions(text);
	return dim.bounding_width;
}

void Face::render_text(const string &text, xhui::Align align, Image &im) {
	auto utf32 = text.utf8_to_utf32();

	//auto glyph_index = FT_Get_Char_Index(face, 'A');
	//msg_write(glyph_index);
	//errpr = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); //load_flags);
	//error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); //render_mode);

	auto dim = get_text_dimensions(text);

	//font_set_font(font_name, font_size);

	/*int nn = 1;
	for (int u: utf32)
		if (u == '\n')
			nn ++;*/

	im.create(dim.bounding_width, dim.bounding_height, color(0,0,0,0));

	int x=0, y = dim.bounding_top_to_line;

	for (int u: utf32) {
		if (u == '\n') {
			x = 0;
			y += dim.line_dy;
			continue;
		}
		int error = FT_Load_Char(face, u, FT_LOAD_RENDER);
		if (error) {
			msg_write("E");
			continue;
		}

		for (int i=0; i<face->glyph->bitmap.width; i++)
			for (int j=0; j<face->glyph->bitmap.rows; j++) {
				float f = (float)face->glyph->bitmap.buffer[i + j*face->glyph->bitmap.width] / 255.0f;
				im.set_pixel(x+face->glyph->bitmap_left+i,y-face->glyph->bitmap_top+j, color(f, 1,1,1));
			}
		x += face->glyph->advance.x >> 6;
	}
}

}

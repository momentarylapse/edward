#include "font.h"
#include <lib/image/image.h>
#include <lib/math/rect.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <lib/os/path.h>
#include <lib/os/filesystem.h>

#if HAS_LIB_FREETYPE2
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace ygfx {

static constexpr float dpi = 96;


#if HAS_LIB_FREETYPE2
Face* load_face(FT_Library ft2, const string& name, bool bold, bool italic, const Array<Path>& directories) {
	Face* face = new Face;
	face->name = name;
	face->bold = bold;
	face->italic = italic;

	int flags = 0;
	string type = "Regular";
	if (bold) {
		type = "Bold";
		flags = FT_STYLE_FLAG_BOLD;
	}

	auto try_load_font = [ft2, face, &type, flags] (const Path& filename) {
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
			if (((int)face->face->style_flags & 0xffff) == flags) {
				//msg_write(str(filename) + "   " + type);
				return true;
			}
		}

		return false;
	};

	string namex = name;
	if (bold)
		namex = name + " Bold";

	Array<Path> candidates;

	// user locations
	for (const auto& dir : directories) {
		candidates.add(dir | format("%s.ttf", name));
		candidates.add(dir | format("%s-%s.ttf", name, type));
	}

	// system locations
#if defined(OS_MAC)
	candidates.add(format("/System/Library/Fonts/%s.ttc", name));
	candidates.add(format("/System/Library/Fonts/Supplemental/%s.ttc", name));
	candidates.add(format("/System/Library/Fonts/Supplemental/%s.ttf", namex));
#elif defined(OS_WINDOWS)
	candidates.add(format("c:/Windows/Fonts/%s.ttf", name.lower()));
#else
	candidates.add(format("/usr/share/fonts/noto/%s-%s.ttf", name, type));
	candidates.add(format("/usr/share/fonts/open-sans/%s-%s.ttf", name, type));
	candidates.add(format("/usr/share/fonts/Adwaita/%s-%s.ttf", name, type));
	candidates.add(format("/usr/share/fonts/opentype/cantarell/%s-VF.otf", name));
	candidates.add(format("/usr/share/fonts/truetype/freefont/%s.ttf", namex.replace(" ", "")));
	candidates.add(format("/usr/share/fonts/truetype/dejavu/%s.ttf", namex.replace(" ", "-")));
#endif

	for (const auto& filename: candidates)
		if (try_load_font(filename))
			return face;
	delete face;
	return nullptr;
}
#endif

void Face::set_size(float size) {
#if HAS_LIB_FREETYPE2
	if (size == current_size)
		return;
	// size: points<<6
	FT_Set_Char_Size(face, 0, int(size*64.0f), (int)dpi, (int)dpi);
	current_size = size;

	FT_Load_Char(face, ' ', FT_LOAD_DEFAULT);
	tab_dx = (int)(face->glyph->advance.x >> 6) * 4;
#endif
}

float Face::units_to_pixel(float units) const {
#if HAS_LIB_FREETYPE2
	// 72 pt/inch
	return units / (float)face->units_per_EM * current_size * dpi / 72.0f;
#else
	return 0;
#endif
}

TextDimensions Face::get_text_dimensions(const string &text) const {
#if HAS_LIB_FREETYPE2
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
		if (u == '\t') {
			x = ((x / tab_dx) + 1) * tab_dx;
			continue;
		}
		int error = FT_Load_Char(face, u, FT_LOAD_DEFAULT);
		if (error) {
			msg_write("E");
			continue;
		}
		//wmax = max(wmax, x + face->glyph->width);
		x += (int)face->glyph->advance.x >> 6;
	}
	dim.bounding_width = (float)max(x, wmax);
	//dim.line_dy = current_font_size - units_to_pixel((float)face->descender);
	dim.line_dy = units_to_pixel((float)face->height);
	dim.ascender = units_to_pixel((float)face->ascender);
	dim.descender = -units_to_pixel((float)face->descender);
	dim.bounding_height = dim.line_dy * (float)dim.num_lines;
	//msg_write(f2s(units_to_pixel((float)face->descender), 3));
	return dim;
#else
	return {};
#endif
}

float TextDimensions::inner_height() const {
	return bounding_height - line_dy + ascender + descender;
}

rect TextDimensions::bounding_box(const vec2& p0) const {
	return rect(p0.x, p0.x + bounding_width, p0.y, p0.y + bounding_height);
}
rect TextDimensions::inner_box(const vec2& p0) const {
	return rect(p0.x, p0.x + bounding_width, p0.y, p0.y + inner_height());
}


float Face::get_text_width(const string &text) const {
	auto dim = get_text_dimensions(text);
	return dim.bounding_width;
}

void Face::render_text(const string &text, Align align, Image &im) {
#if HAS_LIB_FREETYPE2
	auto utf32 = text.utf8_to_utf32();

	//auto glyph_index = FT_Get_Char_Index(face, 'A');
	//msg_write(glyph_index);
	//errpr = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); //load_flags);
	//error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); //render_mode);

	auto dim = get_text_dimensions(text);

	//font_set_font(font_name, font_size);

	im.create((int)dim.bounding_width, (int)dim.bounding_height, color(0,0,0,0));

	int x=0, y = (int)dim.ascender;

	for (int u: utf32) {
		if (u == '\n') {
			x = 0;
			y += (int)dim.line_dy;
			continue;
		}
		if (u == '\t') {
			x = ((x / tab_dx) + 1) * tab_dx;
			continue;
		}
		if (int error = FT_Load_Char(face, u, FT_LOAD_RENDER)) {
			msg_write("E");
			continue;
		}

		for (int i=0; i<face->glyph->bitmap.width; i++)
			for (int j=0; j<face->glyph->bitmap.rows; j++) {
				float f = (float)face->glyph->bitmap.buffer[i + j*face->glyph->bitmap.width] / 255.0f;
				im.set_pixel(x+face->glyph->bitmap_left+i,y-face->glyph->bitmap_top+j, color(f, 1,1,1));
			}
		x += (int)face->glyph->advance.x >> 6;
	}
#endif
}

FontManager::FontManager() {
#if HAS_LIB_FREETYPE2
	auto error = FT_Init_FreeType(&ft2);
	if (error) {
		throw Exception("can not initialize freetype2 library");
	}
#else
	throw Exception("compiled without freetype2 library");
#endif
}

void FontManager::try_load_defaults(const Array<string>& font_names, const Array<string>& font_names_mono) {
	default_font_regular = load_first(font_names, false, false);
	default_font_bold = load_first(font_names, true, false);
	if (default_font_regular and !default_font_bold)
		default_font_bold = default_font_regular;
	if (!default_font_regular)
		msg_error("no font found...");

	default_font_mono_regular = load_first(font_names_mono, false, false);
	default_font_mono_bold = load_first(font_names_mono, true, false);
	if (!default_font_mono_regular)
		default_font_mono_regular = default_font_regular;
	if (!default_font_mono_bold)
		default_font_mono_bold = default_font_mono_regular;
}

Face* FontManager::load(const string& name, bool bold, bool italic) {
	for (auto f: faces)
		if (f->name == name and f->bold == bold and f->italic == italic)
			return f;

	if (auto f = load_face(ft2, name, bold, italic, directories)) {
		faces.add(f);
		return f;
	}
	return nullptr;
}

Face *FontManager::load_first(const Array<string>& font_names, bool bold, bool italic) {
	for (const string& name: font_names)
		if (auto f = load(name, bold, italic))
			return f;
	return nullptr;
}


Face* FontManager::pick(const string& font, bool bold, bool italic) const {
	auto face = default_font_regular;
	if (bold)
		face = default_font_bold;
	if (font == "monospace") {
		if (bold and default_font_mono_bold)
			face = default_font_mono_bold;
		else if (default_font_mono_regular)
			face = default_font_mono_regular;
	}
	return face;
}
}

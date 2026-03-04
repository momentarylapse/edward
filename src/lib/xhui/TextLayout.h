#pragma once

#include <lib/base/base.h>
#include <lib/base/optional.h>
#include <lib/math/vec2.h>
#include <lib/math/rect.h>
#include <lib/image/color.h>

class Painter;

namespace xhui {

struct TextLayout {
	struct Part {
		string text;
		float font_size;
		base::optional<color> col;
		bool bold;
		bool italic;
		vec2 pos;
		rect box;
	};
	Array<Part> parts;
	rect box() const;

	static TextLayout from_format_string(::Painter* p, const string& s, float font_size = -1);
};

// base line will be at y = (pos.y + font_size)
void draw_text_layout(::Painter* p, const vec2& pos, const TextLayout& l, const color& fg);
void draw_text_layout_with_box(::Painter* p, const vec2& pos, const TextLayout& l, const color& fg, const color& bg, float padding=7, float roundness=7);

}

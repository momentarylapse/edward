
#include "Painter.h"
#include "Context.h"
#include <lib/image/image.h>
#include <lib/math/mat4.h>
#include <lib/base/algo.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include <cmath>


namespace ygfx {


Painter::Painter(DrawingHelperData* _aux, const rect& native_area, const rect& area, float _ui_scale, font::Face* _face) {
	aux = _aux;
	if (aux)
		context = aux->context;
	this->_area = area;
	this->native_area = native_area;
	this->native_area_window = native_area;
	width = (int)area.width();
	height = (int)area.height();
	_clip = _area;
	mat_pixel_to_rel = mat4::translation({- 1,- 1, 0}) *  mat4::scale(2.0f / area.width(), 2.0f / area.height(), 1);

	ui_scale = _ui_scale;
	face = _face;

	Painter::set_color(White);
	Painter::set_font("", 16, false, false);

#ifdef USING_VULKAN
	cb = aux->cb;
#endif
}

void Painter::set_color(const color &c) {
	_color = c;
}

/*font::Face* pick_font(const string &font, bool bold, bool italic) {
	font::Face* face;
	if (bold)
		face = default_font_bold;
	else
		face = default_font_regular;
	if (font == "monospace") {
		if (bold and default_font_mono_bold)
			face = default_font_mono_bold;
		else if (default_font_mono_regular)
			face = default_font_mono_regular;
	}
	return face;
}*/

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	if (font != "")
		font_name = font;
	if (font_size > 0)
		font_size = size;
	//face = pick_font(font_name, bold, italic);
	if (face)
		face->set_size(font_size * ui_scale);
}

void Painter::set_font_size(float size) {
	font_size = size;
	if (face)
		face->set_size(size * ui_scale);
}

vec2 Painter::get_str_size(const string &str) {
	const auto& dim = get_cached_text_dimensions(str, face, font_size, ui_scale);
	return {dim.bounding_width / ui_scale, dim.inner_height() / ui_scale};
}

void Painter::set_line_width(float width) {
	line_width = width;
}

void Painter::set_roundness(float radius) {
	corner_radius = radius;
}

void Painter::draw_arc(const vec2& p, float r, float w0, float w1) {
	//float w = (w0 + w1) / 2;
	draw_line({p.x + r * cosf(w0), p.y - r * sinf(w0)}, {p.x + r * cosf(w1), p.y - r * sinf(w1)});
	//draw_line({p.x + r * cos(w), p.y - r * sin(w)}, {p.x + r * cos(w1), p.y - r * sin(w1)});
}

void Painter::draw_circle(const vec2& p, float radius) {
	if (fill) {
		float r0 = corner_radius;
		corner_radius = radius;
		draw_rect({p - vec2(radius, radius), p + vec2(radius, radius)});
		corner_radius = r0;
	} else {
		Array<vec2> points;
		int N = 64;
		for (int i = 0; i <= N; i++) {
			float t = (float)i / (float)N;
			points.add(p + vec2(cos(t * 2 * pi), sin(t * 2 * pi)) * radius);
		}
		draw_lines(points);
	}
}

}



#include "Painter.h"
#include "Context.h"
#include "Theme.h"
#include "draw/font.h"
#include "../image/image.h"
#include "../math/mat4.h"
#include <lib/base/algo.h>
#include "../os/msg.h"

namespace xhui {

void Painter::set_color(const color &c) {
	_color = c;
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	font_name = font;
	font_size = size;
	if (bold)
		face = default_font_bold;
	else
		face = default_font_regular;
	face->set_size(size * ui_scale);
}

void Painter::set_font_size(float size) {
	font_size = size;
	face->set_size(size * ui_scale);
}

vec2 Painter::get_str_size(const string &str) {
	const auto dims = face->get_text_dimensions(str);
	return {dims.bounding_width / ui_scale, dims.inner_height() / ui_scale};
}

void Painter::set_line_width(float width) {
	line_width = width;
}

void Painter::set_roundness(float radius) {
	corner_radius = radius;
}

void Painter::draw_arc(const vec2& p, float r, float w0, float w1) {
	//float w = (w0 + w1) / 2;
	draw_line({p.x + r * (float)cos(w0), p.y - r * (float)sin(w0)}, {p.x + r * (float)cos(w1), p.y - r * (float)sin(w1)});
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

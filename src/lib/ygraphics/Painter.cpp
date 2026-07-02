
#include "Painter.h"
#include "Context.h"
#include <lib/image/image.h>
#include <lib/math/mat4.h>
#include <lib/base/algo.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include <cmath>

#include "TextCache.h"


namespace ygfx {

Painter::Painter(DrawingHelperData* _aux, FontManager* fm, TextCache* tc, const rect& native_area, const rect& area) {
	aux = _aux;
	font_manager = fm;
	text_cache = tc;
	this->_area = area;
	this->native_area = native_area;
	this->native_area_window = native_area;
	width = (int)area.width();
	height = (int)area.height();
	_clip = _area;
	ui_scale.x = native_area.height() / area.height();
	ui_scale.y = native_area.width() / area.width();
	if (font_manager)
		face = font_manager->default_font_regular;
	mat_pixel_to_rel = mat4::translation({-1,-1, 0}) *  mat4::scale(2/area.width(), 2/area.height(), 1);
#ifdef USING_OPENGL
	// direct to window? -> flip etc
	mat_pixel_to_rel = mat4::translation(vec3(-1,1,0)) * mat4::scale(2/area.width(), -2/area.height(), 2);
#endif

	if (aux) {
		context = aux->context;
		Painter::set_color(White);
		Painter::set_font("", min(16.0f, 100.0f / ui_scale.y), false, false);
#ifdef USING_VULKAN
		cb = aux->cb;
#endif
	}
}

void Painter::set_color(const color &c) {
	_color = context->color_input_to_shaders(c);
}

void Painter::set_font_face(Face *f) {
	face = f;
}

void Painter::set_texture(Texture* tex) {
	user_texture = tex;
}

void Painter::set_shader(Shader* s) {
	user_shader = s;
}

void Painter::set_shader_data(const Any& data) {
	user_shader_data = data;
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	if (font != "")
		font_name = font;
	if (font_size > 0)
		font_size = size;
	face = font_manager->pick(font_name, bold, italic);
	if (face)
		face->set_size(font_size * ui_scale.y);
}

void Painter::set_font_size(float size) {
	font_size = size;
	if (face)
		face->set_size(size * ui_scale.y);
}

vec2 Painter::get_str_size(const string &str) {
	const auto& dim = text_cache->get_dimensions(str, face, font_size, ui_scale.y);
	return {dim.bounding_width / ui_scale.x, dim.inner_height() / ui_scale.y};
}

void Painter::set_line_width(float width) {
	line_width = width;
}

void Painter::set_roundness(float radius) {
	corner_radius = radius;
}

void Painter::draw_arc(const vec2& p, float r, float w0, float w1) {
	if (w0 > w1)
		std::swap(w0, w1);
	if (r * ui_scale.y > 4) {
		int n = clamp((int)(r * ui_scale.y / 3), 2, 30);
		Array<vec2> points;
		points.resize(n + 1);
		for (int i=0; i<=n; i++) {
			float w = w0 + (w1 - w0) * (float)i / (float)n;
			points[i] = {p.x + r * cosf(w), p.y - r * sinf(w)};
		}
		draw_lines(points);
	} else {
		draw_line({p.x + r * cosf(w0), p.y - r * sinf(w0)}, {p.x + r * cosf(w1), p.y - r * sinf(w1)});
	}
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

static void add_vb_line(Array<VertexX>& vertices, const vec2& a, const vec2& b, float line_width, const color& col) {
	vec2 dir = (b - a).normalized();
	vec2 r = dir.ortho() * line_width / 2;
	dir *= line_width * 0.2f;
	vec2 a0 = a - r - dir;
	vec2 a1 = a + r - dir;
	vec2 b0 = b - r + dir;
	vec2 b1 = b + r + dir;
	vertices.add({{a0.x, a0.y, 0}, v_0, 0,0, col});
	vertices.add({{a1.x, a1.y, 0}, v_0, 0,0, col});
	vertices.add({{b0.x, b0.y, 0}, v_0, 0,0, col});
	vertices.add({{b0.x, b0.y, 0}, v_0, 0,0, col});
	vertices.add({{a1.x, a1.y, 0}, v_0, 0,0, col});
	vertices.add({{b1.x, b1.y, 0}, v_0, 0,0, col});
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	aux->projection_matrix = &mat_pixel_to_rel;
	/*if (a.x == b.x) {
		fill_rect(context, rect(a.x + 0.5f - line_width/2, a.x + 0.5f + line_width/2, a.y, b.y), _color, 0, 0);
	} else if (a.y == b.y) {
		fill_rect(context, rect(a.x, b.x, a.y + 0.5f - line_width/2, a.y + 0.5f + line_width/2), _color, 0, 0);
	} else {*/
	// NO geometry shaders on M1... :(
	// CPU lines then...

	Array<VertexX> p;
	add_vb_line(p, a, b, line_width, White);
	draw_simple(aux, p, mat4::ID, _color, false, _color.a < 1);
	//}
}

void Painter::draw_lines(const Array<vec2> &p) {
	aux->projection_matrix = &mat_pixel_to_rel;
	Array<VertexX> vertices;
	if (contiguous) {
		for (int i=0; i<p.num-1; i++)
			add_vb_line(vertices, p[i], p[i+1], line_width, White);
	} else {
		for (int i=0; i<p.num-1; i+=2)
			add_vb_line(vertices, p[i], p[i+1], line_width, White);
	}
	draw_simple(aux, vertices, mat4::ID, _color, false, _color.a < 1);
}

}


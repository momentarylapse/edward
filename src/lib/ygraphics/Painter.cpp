
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


Array<TextCache> text_caches;

TextCache& get_text_cache(DrawingHelperData* aux, const string& text, font::Face* face, float font_size, float ui_scale) {
	for (auto& tc: text_caches)
		if (tc.text == text and tc.face == face and tc.font_size == font_size) {
			tc.age = 0;
			return tc;
		}

	TextCache* tc = nullptr;
	for (auto& _tc: text_caches)
		if (_tc.age > 5)
			tc = &_tc;
	if (!tc) {
		text_caches.add({});
		tc = &text_caches.back();
#if HAS_LIB_VULKAN
		tc->dset = aux->pool->create_set(aux->shader);
#endif
		tc->texture = new ygfx::Texture();
	}

	tc->text = text;
	tc->font_size = font_size;
	tc->face = face;
	tc->age = 0;
	Image im;
	face->render_text(text, font::Align::LEFT, im);
	tc->texture->write(im);
	tc->texture->set_options("minfilter=nearest,wrap=clamp");
	// fractional coordinates (especially with ui_scale)... not sure what to do m(-_-)m
//	tc->texture->set_options("minfilter=nearest,magfilter=nearest,wrap=clamp");

	face->set_size(font_size * ui_scale);
	tc->dimensions = face->get_text_dimensions(text);

#if HAS_LIB_VULKAN
	tc->dset->set_texture(0, tc->texture);
	tc->dset->update();
#endif
	return *tc;
}



struct TextDimensionsCache {
	string text;
	font::Face* face;
	float font_size;
	int age;
	font::TextDimensions dimensions;
};

Array<TextDimensionsCache> text_dimensions_caches;

void iterate_text_caches() {
	for (auto& tc: text_caches)
		tc.age ++;
	for (auto& tc: text_dimensions_caches)
		tc.age ++;
}


font::TextDimensions& get_cached_text_dimensions(const string& text, font::Face* face, float font_size, float ui_scale) {
	// already in cache?
	for (auto& tc: text_dimensions_caches)
		if (tc.text == text and tc.face == face and tc.font_size == font_size) {
			tc.age = 0;
			return tc.dimensions;
		}

	// no? -> calculate now
	TextDimensionsCache* tc = nullptr;
	for (auto& _tc: text_dimensions_caches)
		if (_tc.age > 5)
			tc = &_tc;
	if (!tc) {
		text_dimensions_caches.add({});
		tc = &text_dimensions_caches.back();
	}
	tc->text = text;
	tc->font_size = font_size;
	tc->face = face;
	tc->age = 0;
	face->set_size(font_size * ui_scale);
	tc->dimensions = face->get_text_dimensions(text);
	return tc->dimensions;
}

}



#include "Painter.h"
#include "Context.h"
#include "Theme.h"
#include <lib/image/image.h>
#include <lib/math/mat4.h>
#include <lib/base/algo.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include <lib/ygraphics/Context.h>
#include <cmath>

namespace xhui {

Painter::Painter(Context* c, Window* window, const rect& native_area, const rect& area) : ygfx::Painter(c ? c->aux : nullptr, native_area, area, 1.0f, default_font_regular) {
	this->context = c;
	this->_area = area;
	this->native_area = native_area;
	this->native_area_window = native_area;
	width = (int)area.width();
	height = (int)area.height();
	_clip = _area;
	mat_pixel_to_rel = mat4::translation({- 1,- 1, 0}) *  mat4::scale(2.0f / (float)width, 2.0f / (float)height, 1);

	if (window) {
		ui_scale = window->ui_scale;
		context = window->context;
		face = default_font_regular;

		Painter::set_color(Theme::_default.text);
		Painter::set_font(Theme::_default.font_name /*"CAC Champagne"*/, Theme::_default.font_size, false, false);

#ifdef USING_VULKAN
		cb = context->current_command_buffer();
#endif
	}
}

font::Face* pick_font(const string &font, bool bold, bool italic) {
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
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	if (font != "")
		font_name = font;
	if (font_size > 0)
		font_size = size;
	face = pick_font(font_name, bold, italic);
	face->set_size(font_size * ui_scale);
}

void Painter::set_font_size(float size) {
	font_size = size;
	face->set_size(size * ui_scale);
}


Array<TextCache> text_caches;

TextCache& get_text_cache(Context* context, const string& text, font::Face* face, float font_size, float ui_scale) {
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
		tc->__dset = context->aux->pool->create_set(context->aux->shader);
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
	tc->__dset->set_texture(0, tc->texture);
	tc->__dset->update();
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

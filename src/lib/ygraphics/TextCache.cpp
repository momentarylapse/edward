//
// Created by michi on 6/30/26.
//

#include "TextCache.h"
#include "Context.h"
#include "graphics-impl.h"
#include "font.h"
#include <lib/image/image.h>


namespace ygfx {


struct CachedTextDimensions {
	string text;
	Face* face;
	float font_size;
	int age;
	TextDimensions dimensions;
};

TextCache::TextCache(DrawingHelperData* _aux) {
	aux = _aux;
}

TextCache::~TextCache() = default;

void TextCache::iterate() {
	for (auto& tc: cached_texts)
		tc.age ++;
	for (auto& tc: cached_texts_dimensions)
		tc.age ++;
}

CachedText& TextCache::get(const string& text, Face* face, float font_size, float ui_scale) {
	font_size *= ui_scale;
	for (auto& tc: cached_texts)
		if (tc.text == text and tc.face == face and tc.font_size == font_size) {
			tc.age = 0;
			return tc;
		}

	CachedText* tc = nullptr;
	for (auto& _tc: cached_texts)
		if (_tc.age > 5)
			tc = &_tc;
	if (!tc) {
		cached_texts.add({});
		tc = &cached_texts.back();
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
	face->set_size(font_size);
	face->render_text(text, Align::LEFT, im);
	if (im.width * im.height > 0) {
		tc->texture->write(im);
		tc->texture->set_options("minfilter=nearest,wrap=clamp");
		// fractional coordinates (especially with ui_scale)... not sure what to do m(-_-)m
		tc->texture->set_options("minfilter=nearest,magfilter=nearest,wrap=clamp");
	}

	tc->dimensions = face->get_text_dimensions(text);

#if HAS_LIB_VULKAN
	tc->dset->set_texture(0, tc->texture);
	tc->dset->update();
#endif
	return *tc;
}


TextDimensions& TextCache::get_dimensions(const string& text, Face* face, float font_size, float ui_scale) {
	font_size *= ui_scale;
	// already in cache?
	for (auto& tc: cached_texts_dimensions)
		if (tc.text == text and tc.face == face and tc.font_size == font_size) {
			tc.age = 0;
			return tc.dimensions;
		}

	// no? -> calculate now
	CachedTextDimensions* tc = nullptr;
	for (auto& _tc: cached_texts_dimensions)
		if (_tc.age > 5)
			tc = &_tc;
	if (!tc) {
		cached_texts_dimensions.add({});
		tc = &cached_texts_dimensions.back();
	}
	tc->text = text;
	tc->font_size = font_size;
	tc->face = face;
	tc->age = 0;
	face->set_size(font_size);
	tc->dimensions = face->get_text_dimensions(text);
	return tc->dimensions;
}
} // ygfx
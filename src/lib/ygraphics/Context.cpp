//
// Created by michi on 9/2/25.
//

#include "Context.h"
#include "graphics-impl.h"
#include <lib/image/image.h>

namespace ygfx {

color Context::color_input_to_shaders(const color& c) const {
	if (color_space_input == color_space_shaders)
		return c;
	if (color_space_input == ColorSpace::SRGB and color_space_shaders == ColorSpace::Linear)
		return c.srgb_to_linear();
	if (color_space_input == ColorSpace::Linear and color_space_shaders == ColorSpace::SRGB)
		return c.linear_to_srgb();
	return c;
}

void Context::_create_default_textures() {
	tex_white = new Texture();
	tex_black = new Texture();
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));
	tex_white->_pointer_ref_counter = 999999999;
}

TextCache& DrawingHelperData::get_text_cache(const string& text, font::Face* face, float font_size, float ui_scale) {
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
		tc->dset = pool->create_set(shader);
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
	tc->texture->set_options("minfilter=nearest,magfilter=nearest,wrap=clamp");

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

void DrawingHelperData::iterate_text_caches() {
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

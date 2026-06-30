//
// Created by michi on 6/30/26.
//

#pragma once

#include <lib/base/base.h>
#include "graphics-fwd.h"
#include "font.h"

namespace ygfx {

struct DrawingHelperData;

struct CachedText {
	string text;
	Face* face;
	float font_size;
	int age;
	Texture* texture;
#if HAS_LIB_VULKAN
	vulkan::DescriptorSet* dset;
#endif
	TextDimensions dimensions;
};

struct CachedTextDimensions;

class TextCache {
public:
	explicit TextCache(DrawingHelperData* aux);
	~TextCache();
	void iterate();

	CachedText& get(const string& text, Face* face, float font_size, float ui_scale);
	TextDimensions& get_dimensions(const string& text, Face* face, float font_size, float ui_scale);

	DrawingHelperData* aux;
	Array<CachedText> cached_texts;
	Array<CachedTextDimensions> cached_texts_dimensions;
};

}

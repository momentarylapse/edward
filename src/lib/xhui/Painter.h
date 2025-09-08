#pragma once

#include "xhui.h"
#include <lib/image/ImagePainter.h>
#include <lib/math/vec2.h>
#include <lib/math/mat4.h>
#include <lib/ygraphics/font.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/ygraphics/Painter.h>


#if HAS_LIB_VULKAN
namespace vulkan {
	class CommandBuffer;
	class DescriptorSet;
}
#endif

namespace font {
	struct Face;
}


namespace xhui {

class Window;
class Context;
struct XImage;

class Painter : public ygfx::Painter {
public:
	explicit Painter(Context* c, Window *w, const rect& native_area, const rect& area);
	//virtual ~Painter();

	void set_font(const string &font, float size, bool bold, bool italic) override;
	void set_font_size(float size) override;
	void draw_ximage(const rect& r, const XImage *image);

	Context* context = nullptr;
};


struct TextCache {
	string text;
	font::Face* face;
	float font_size;
	int age;
	ygfx::Texture* texture;
#if HAS_LIB_VULKAN
	vulkan::DescriptorSet* __dset;
#endif
	font::TextDimensions dimensions;
};

TextCache& get_text_cache(Context* context, const string& text, font::Face* face, float font_size, float ui_scale);
void iterate_text_caches();
font::TextDimensions& get_cached_text_dimensions(const string& text, font::Face* face, float font_size, float ui_scale);

}

#pragma once

#include "xhui.h"
#include "draw/font.h"
#include <lib/image/ImagePainter.h>
#include <lib/math/vec2.h>
#include <lib/ygraphics/graphics-fwd.h>


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

class Painter : public ::Painter {
public:
	explicit Painter(Window *w);
	//virtual ~Painter();


	void end();
	void set_color(const color &c) override;
	void set_font(const string &font, float size, bool bold, bool italic) override;
	void set_font_size(float size) override;
	void set_antialiasing(bool enabled) override {}
	void set_line_width(float w) override;
	void set_line_dash(const Array<float> &dash, float offset) override {}
	void set_roundness(float radius) override;
	void set_fill(bool fill) override { this->fill = fill; }
	void set_clip(const rect &r) override;
	void draw_point(const vec2 &p) override {}
	void draw_line(const vec2 &a, const vec2 &b) override;
	void draw_lines(const Array<vec2> &p) override;
	void draw_polygon(const Array<vec2> &p) override {}
	void draw_rect(const rect &r) override;
	void draw_circle(const vec2 &p, float radius) override;
	void draw_str(const vec2 &p, const string &str) override;
	vec2 get_str_size(const string &str) override;
	void draw_image(const vec2 &p, const ::Image *image) override {}
	void draw_mask_image(const vec2 &p, const ::Image *image) override {}
	void draw_ximage(const rect& r, const XImage *image);

	void draw_arc(const vec2& p, float r, float w0, float w1);

	void clear(const color &c);
	void set_transform(float rot[], const vec2 &offset) override;

	rect clip() const override {
		return _clip;
	}
	rect area() const override {
		return _area;
	}

	float ui_scale;
	rect _area;
	rect native_area;
	rect native_area_window;

	color _color = White;;
	string font_name;
	rect _clip;
	//Array<float> dash;
	//float dash_offset;
	float line_width = 1;
	//bool anti_aliasing;
	float offset_x = 0, offset_y = 0;
	float corner_radius = 0;
	float softness = 0;
	bool fill = true;
	font::Face* face;

	bool accumulate_alpha = false;

	Window *window;

	Context* context = nullptr;
#if HAS_LIB_VULKAN
	vulkan::CommandBuffer* cb;
#endif
};


struct TextCache {
	string text;
	font::Face* face;
	float font_size;
	int age;
	ygfx::Texture* texture;
#if HAS_LIB_VULKAN
	vulkan::DescriptorSet* dset;
#endif
	font::TextDimensions dimensions;
};

TextCache& get_text_cache(Context* context, const string& text, font::Face* face, float font_size, float ui_scale);
void iterate_text_caches();
font::TextDimensions& get_cached_text_dimensions(const string& text, font::Face* face, float font_size, float ui_scale);

}

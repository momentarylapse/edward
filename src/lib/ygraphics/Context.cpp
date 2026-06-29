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

//--------------------------------------------------------



void DrawingHelperData::create_basic() {

	vb = new VertexBuffer("3f,3f,2f,4f");
	//vb->create_quad(rect::ID, rect::ID);
	Array<VertexX> vv = {{vec3(0,0,0), {0,0,0}, 0, 0, White},
						 {vec3(0,1,0), {0,0,0}, 0, 1, White},
						 {vec3(1,1,0), {0,0,0}, 1, 1, White},
						 {vec3(0,0,0), {0,0,0}, 0, 0, White},
						 {vec3(1,1,0), {0,0,0}, 1, 1, White},
						 {vec3(1,0,0), {0,0,0}, 1, 0, White}};
	vb->update(vv);

	auto mm = context->create_shader(
			R"foodelim(
<Layout>
	name = ui-data
</Layout>
<Module>

#ifdef vulkan
layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
};
#else
uniform mat4 matrix;
uniform vec4 color;
uniform vec2 size;
uniform float radius = 0;
uniform float softness = 0;
#endif

layout(binding=0) uniform sampler2D tex0;

</Module>
)foodelim");

	shader = context->create_shader(
			R"foodelim(
<Layout>
	version = 430
	bindings = [[sampler]]
	pushsize = 96
</Layout>
<VertexShader>

#import ui-data

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_color;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

void main() {
	gl_Position = matrix * vec4(in_position, 1);
	out_uv = in_uv;
	out_color = in_color;
}
</VertexShader>
<FragmentShader>

#import ui-data

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= color * in_color;
}
</FragmentShader>
)foodelim");

	shader_round  = context->create_shader(
			R"foodelim(
<Layout>
	version = 430
	bindings = [[sampler]]
	pushsize = 96
</Layout>
<VertexShader>

#import ui-data

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_color;

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

void main() {
	gl_Position = matrix * vec4(in_position, 1);
	out_uv = in_uv;
	out_color = in_color;
}
</VertexShader>
<FragmentShader>

#import ui-data

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= color * in_color;
	if (softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-softness-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius) / softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius), 0, 1);
	}
}
</FragmentShader>
)foodelim");

	_create_basic_internal();

}

VertexBuffer* DrawingHelperData::get_line_vb(bool with_color) {
	if (with_color) {
		if (num_line_vbs_with_color_used < line_vbs_with_color.num)
			return line_vbs_with_color[num_line_vbs_with_color_used ++];

		auto _vb = new VertexBuffer("3f,3f,2f,4f");
		line_vbs_with_color.add(_vb);
		num_line_vbs_with_color_used ++;
		return _vb;
	} else {
		if (num_line_vbs_used < line_vbs.num)
			return line_vbs[num_line_vbs_used ++];

		auto _vb = new VertexBuffer("3f,3f,2f");
		line_vbs.add(_vb);
		num_line_vbs_used ++;
		return _vb;
	}
}

TextCache& DrawingHelperData::get_text_cache(const string& text, font::Face* face, float font_size, float ui_scale) {
	font_size *= ui_scale;
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
	face->set_size(font_size);
	face->render_text(text, font::Align::LEFT, im);
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
	font_size *= ui_scale;
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
	face->set_size(font_size);
	tc->dimensions = face->get_text_dimensions(text);
	return tc->dimensions;
}

}

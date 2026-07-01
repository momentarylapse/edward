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
		vec2 pp = ((abs(in_uv - 0.5) - 0.5) * size + radius + softness);
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius) / softness, 0, 1);
	} else {
		vec2 pp = ((abs(in_uv - 0.5) - 0.5) * size + radius);
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

}

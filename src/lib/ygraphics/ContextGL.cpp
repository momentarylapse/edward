#if HAS_LIB_GL

#include "Context.h"
#include "graphics-impl.h"

namespace ygfx {

DrawingHelperData::DrawingHelperData(Context* c) {
	context = c;
}


void DrawingHelperData::reset_frame() {
	//descriptor_sets_used = 0;
	num_line_vbs_used = 0;
	num_line_vbs_with_color_used = 0;
}

void DrawingHelperData::_create_basic_internal() {


#if 0
	<VertexShader>
	#version 330 core
	#extension GL_ARB_separate_shader_objects : enable

	struct Matrix { mat4 model, view, project; };
	/*layout(binding = 0)*/ uniform Matrix matrix;

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec2 in_uv;

	layout(location = 0) out vec2 out_uv;

	void main() {
		gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
		out_uv = in_uv;
	}

	</VertexShader>
#endif

	shader->filename = "-my-shader-";
	shader_round->filename = "-my-shader-round-";

	tex_xxx = new Texture();
	tex_text = new Texture();

}

Context::Context(nix::Context* _ctx) {
	ctx = _ctx;
	color_space_shaders = ColorSpace::Linear;
	color_space_input = ColorSpace::SRGB;
}

DrawingHelperData* Context::_create_auxiliary_stuff() {
	auto aux = new DrawingHelperData(this);
	aux->create_basic();
	return aux;
}

void Context::make_current() {
}

Shader *Context::create_shader(const string &source) const {
	return ctx->create_shader(source);
}

}

#endif

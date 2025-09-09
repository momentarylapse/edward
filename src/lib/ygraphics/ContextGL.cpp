#if HAS_LIB_GL

#include "Context.h"
#include "graphics-impl.h"

namespace ygfx {

DrawingHelperData::DrawingHelperData(Context* c) {
	context = c;
}


void DrawingHelperData::reset_frame() {
	//descriptor_sets_used = 0;
}

void DrawingHelperData::create_basic() {

	vb = new VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID, rect::ID);



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

	shader = context->ctx->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
layout(binding=0) uniform sampler2D tex0;
uniform vec4 _color_;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
}
</FragmentShader>
)foodelim");
	shader->filename = "-my-shader-";


	shader_round  = context->ctx->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
uniform sampler2D tex0;
uniform vec4 _color_;
uniform vec2 size;
uniform float radius = 0;
uniform float softness = 0;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
	if (softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-softness-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius) / softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius), 0, 1);
	}
	//out_color = vec4(length(pp)/radius, 0, 1, 1);
}
</FragmentShader>
)foodelim");
	shader_round->filename = "-my-shader-round-";

	tex_xxx = new Texture();
	tex_text = new Texture();

}

Context::Context(nix::Context* _ctx) {
	ctx = _ctx;
}

DrawingHelperData* Context::_create_auxiliary_stuff() {
	auto aux = new DrawingHelperData(this);
	aux->create_basic();
	return aux;
}

void Context::make_current() {
}

}

#endif

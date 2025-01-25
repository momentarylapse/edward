//
// Created by michi on 25.01.25.
//

#include "DrawingHelper.h"
#include <y/graphics-impl.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>

#include "MultiView.h"

DrawingHelper::DrawingHelper(xhui::ContextVulkan* ctx) {
	context = ctx;
	shader = vulkan::Shader::create(
		R"foodelim(
<Layout>
	version = 430
	bindings = [[sampler]]
	pushsize = 96
</Layout>
<VertexShader>

//#extension GL_ARB_separate_shader_objects : enable

layout(push_constant, std430) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_pos; // camera space
layout(location = 1) out vec2 out_uv;

void main() {
	gl_Position = params.matrix * vec4(in_position, 1);
	out_pos = gl_Position;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>

layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex0;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= params.color;
}
</FragmentShader>
)foodelim");
	dset = ctx->pool->create_set(shader);
	dset->set_texture(0, ctx->tex_white);
	dset->update();

	pipeline = new vulkan::GraphicsPipeline(shader, context->render_pass, 0, "triangles", context->vb);
	//pipeline->set_z(false, false);
	pipeline->set_culling(vulkan::CullMode::NONE);
	pipeline->rebuild();
}

void DrawingHelper::set_window(MultiViewWindow* win) {
	window = win;
}


void DrawingHelper::set_color(const color& color) {
	_color = color;
}


static void add_vb_line(Array<Vertex1>& vertices, const vec3& a, const vec3& b, MultiViewWindow* win, float line_width) {
	float w = win->area.width();
	float h = win->area.height();
	vec2 ba_pixel = vec2((b.x - a.x) * w, (b.y - a.y) * h);
	vec2 dir_pixel = ba_pixel.normalized();
	vec2 o_pixel = dir_pixel.ortho();
	vec3 r = vec3(o_pixel.x/ w, o_pixel.y/ h, 0) * (line_width / 2);
	vec3 a0 = a - r;
	vec3 a1 = a + r;
	vec3 b0 = b - r;
	vec3 b1 = b + r;
	vertices.add({a0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b1, v_0, 0,0});
}

void DrawingHelper::draw_lines(const Array<vec3>& points, float width) {
	auto vb = context->get_line_vb();
	Array<Vertex1> vertices;
	mat4 m = window->projection * window->view;
	for (int i=0; i<points.num-1; i++)
		add_vb_line(vertices, m.project(points[i]), m.project(points[i+1]), window, width);
	vb->update(vertices);

	struct Parameters {
		mat4 matrix;
		color col;
		vec2 size;
		float radius, softness;
	};

	Parameters params;
	params.matrix = mat4::ID;
	params.col = _color;
	params.size = {1000,1000};//(float)width, (float)height};
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	auto cb = context->current_command_buffer();
	cb->bind_pipeline(pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, dset);
	cb->draw(vb);
}

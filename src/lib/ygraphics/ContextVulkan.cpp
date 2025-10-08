#if HAS_LIB_VULKAN

#include "Context.h"
#include "../os/msg.h"
#include "../image/image.h"
#include "graphics-impl.h"

namespace ygfx {

DrawingHelperData::DrawingHelperData(Context* c) {
	context = c;
}

void DrawingHelperData::reset_frame() {
	descriptor_sets_used = 0;
	num_line_vbs_used = 0;
	num_line_vbs_with_color_used = 0;
}


vulkan::DescriptorSet* DrawingHelperData::get_descriptor_set(Texture* texture) {
	vulkan::DescriptorSet* dset = nullptr;
	if (descriptor_sets_used < descriptor_sets.num) {
		dset = descriptor_sets[descriptor_sets_used ++];
	} else {
		dset = pool->create_set(shader);
		descriptor_sets.add(dset);
	}
	dset->set_texture(0, texture);
	dset->update();
	return dset;
}

vulkan::VertexBuffer* DrawingHelperData::get_line_vb(bool with_color) {
	if (with_color) {
		if (num_line_vbs_with_color_used < line_vbs_with_color.num)
			return line_vbs_with_color[num_line_vbs_with_color_used ++];

		auto vb = new vulkan::VertexBuffer("3f,3f,2f,4f");
		line_vbs_with_color.add(vb);
		num_line_vbs_with_color_used ++;
		return vb;
	} else {
		if (num_line_vbs_used < line_vbs.num)
			return line_vbs[num_line_vbs_used ++];

		auto vb = new vulkan::VertexBuffer("3f,3f,2f");
		line_vbs.add(vb);
		num_line_vbs_used ++;
		return vb;
	}
}

void DrawingHelperData::create_basic() {
	pool = new vulkan::DescriptorPool("buffer:4096,sampler:4096", 65536);

	vb = new vulkan::VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID, rect::ID);

	try {
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
	/*out_color = texture(tex0, in_uv);
	out_color *= params.color;*/
#if 0
	out_color = params.color;
	out_color.rg = fract(in_uv * params.size);
	return;
#endif

	out_color = texture(tex0, in_uv);
	out_color *= params.color;
	if (params.softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * params.size - (0.5*params.size-params.softness-params.radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - params.radius) / params.softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * params.size - (0.5*params.size-params.radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - params.radius), 0, 1);
	}
}
</FragmentShader>
)foodelim");
		dset = pool->create_set(shader);
		dset->set_texture(0, context->tex_white);
		dset->update();


		shader_lines = vulkan::Shader::create(
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

//layout(location = 0) out vec4 out_pos; // camera space
//layout(location = 1) out vec2 out_uv;

void main() {
	gl_Position = params.matrix * vec4(in_position, 1);
	//out_pos = gl_Position;
	//out_uv = vec2(0,0);
}
</VertexShader>
<GeometryShader>

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

/*layout(location=0) in vec4 in_color[];
layout(location=1) in vec2 in_uv[];
layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_uv;*/

layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

void main() {
	/*float w0 = gl_in[0].gl_Position.w;
	float w1 = gl_in[1].gl_Position.w;
	vec2 d = (gl_in[1].gl_Position.xy / w1 - gl_in[0].gl_Position.xy / w0);
	d = vec2(d.x * params.size.x/2, d.y * params.size.y/2);
	d = vec2(d.y, -d.x) / length(d) * params.radius/2 * 10;
	d = vec2(d.x / params.size.x*2, d.y / params.size.y*2);*/

	//gl_Position = gl_in[0].gl_Position - vec4(d*w0,0,0);
	gl_Position = vec4(0,0,0,1);
	//out_color = vec4(1,0,0,1);//in_color[0];
	//out_uv = vec2(0,0);
	EmitVertex();

	//gl_Position = gl_in[1].gl_Position - vec4(d*w1,0,0) + vec4(w1*200,0,0,0);
	gl_Position = vec4(1,0,0,1);
	//out_color = vec4(1,0,0,1);//in_color[1];
	//out_uv = vec2(0,0);
	EmitVertex();

	//gl_Position = gl_in[0].gl_Position + vec4(d*w0,0,0);
	gl_Position = vec4(0,1,0,1);
	//out_color = vec4(1,0,0,1);//in_color[0];
	//out_uv = vec2(0,0);
	EmitVertex();

	//gl_Position = gl_in[1].gl_Position + vec4(d*w1,0,0);
	gl_Position = vec4(1,1,0,1);
	//out_color = vec4(1,0,0,1);//in_color[1];
	//out_uv = vec2(0,0);
	EmitVertex();

	EndPrimitive();
}

</GeometryShader>
<FragmentShader>

layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

//layout(location = 0) in vec4 in_color;
//layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex0;

void main() {
	//out_color = texture(tex0, in_uv);
	//out_color *= in_color;
	out_color = params.color;

	/*out_color = texture(tex0, in_uv);
	out_color *= params.color;
	if (params.softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * params.size - (0.5*params.size-params.softness-params.radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - params.radius) / params.softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * params.size - (0.5*params.size-params.radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - params.radius), 0, 1);
	}*/
}
</FragmentShader>
)foodelim");
		dset_lines = pool->create_set(shader_lines);
		dset_lines->set_texture(0, context->tex_white);
		dset_lines->update();

	} catch (Exception& e) {
		msg_error(e.message());
		throw;
	}
}


void DrawingHelperData::rebuild(RenderPass* render_pass) {
	pipeline = new vulkan::GraphicsPipeline(shader, render_pass, 0, vulkan::PrimitiveTopology::TRIANGLES, vb);
	pipeline->set_dynamic({"scissor"});
	pipeline->set_z(false, false);
	pipeline->set_culling(vulkan::CullMode::NONE);
	pipeline->rebuild();

	pipeline_alpha = new vulkan::GraphicsPipeline(shader, render_pass, 0, vulkan::PrimitiveTopology::TRIANGLES, vb);
	pipeline_alpha->set_dynamic({"scissor"});
	pipeline_alpha->set_z(false, false);
	pipeline_alpha->set_culling(vulkan::CullMode::NONE);
	pipeline_alpha->set_blend(vulkan::Alpha::SOURCE_ALPHA, vulkan::Alpha::SOURCE_INV_ALPHA);
	pipeline_alpha->rebuild();

	pipeline_lines = new vulkan::GraphicsPipeline(shader_lines, render_pass, 0, vulkan::PrimitiveTopology::LINES, "3f");
	pipeline_lines->set_dynamic({"scissor"});
	pipeline_lines->set_z(false, false);
	pipeline_lines->set_culling(vulkan::CullMode::NONE);
	pipeline_lines->rebuild();
}

Context::Context(vulkan::Instance* _instance, vulkan::Device* _device) {
	instance = _instance;
	device = _device;
	color_space_shaders = ColorSpace::Linear;
	color_space_input = ColorSpace::SRGB;
	//num_line_vbs_used = 0;
}

Context::~Context() {
	delete device;
	delete instance;
}

DrawingHelperData* Context::_create_auxiliary_stuff() {
	//device->create_query_pool(MAX_TIMESTAMP_QUERIES);
	auto aux = new DrawingHelperData(this);
	aux->create_basic();
	return aux;
}

void Context::make_current() {
	vulkan::default_device = device;
}


}

#endif

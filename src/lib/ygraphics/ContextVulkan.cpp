#if HAS_LIB_VULKAN

#include "Context.h"
#include "font.h"
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

void DrawingHelperData::_create_basic_internal() {
	pool = new vulkan::DescriptorPool("buffer:4096,sampler:4096", 65536);

	try {

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
};

layout(location = 0) in vec3 in_position;

//layout(location = 0) out vec4 out_pos; // camera space
//layout(location = 1) out vec2 out_uv;

void main() {
	gl_Position = matrix * vec4(in_position, 1);
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
	pipeline = new vulkan::GraphicsPipeline(shader, render_pass, 0, PrimitiveTopology::TRIANGLES, vb);
	pipeline->set_dynamic({"scissor"});
	pipeline->set_z(false, false);
	pipeline->set_culling(CullMode::NONE);
	pipeline->rebuild();

	pipeline_z = new vulkan::GraphicsPipeline(shader, render_pass, 0, PrimitiveTopology::TRIANGLES, vb);
	pipeline_z->set_dynamic({"scissor"});
	pipeline_z->set_z(true, true);
	pipeline_z->set_culling(CullMode::NONE);
	pipeline_z->rebuild();

	pipeline_alpha = new vulkan::GraphicsPipeline(shader, render_pass, 0, PrimitiveTopology::TRIANGLES, vb);
	pipeline_alpha->set_dynamic({"scissor"});
	pipeline_alpha->set_z(true, false);
	pipeline_alpha->set_culling(CullMode::NONE);
	pipeline_alpha->set_blend(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA);
	pipeline_alpha->rebuild();

	pipeline_round = new vulkan::GraphicsPipeline(shader_round, render_pass, 0, PrimitiveTopology::TRIANGLES, vb);
	pipeline_round->set_dynamic({"scissor"});
	pipeline_round->set_z(false, false);
	pipeline_round->set_culling(CullMode::NONE);
	pipeline_round->set_blend(Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA);
	pipeline_round->rebuild();

	pipeline_lines = new vulkan::GraphicsPipeline(shader_lines, render_pass, 0, PrimitiveTopology::LINES, "3f");
	pipeline_lines->set_dynamic({"scissor"});
	pipeline_lines->set_z(false, false);
	pipeline_lines->set_culling(CullMode::NONE);
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

Shader *Context::create_shader(const string &source) const {
	return vulkan::Shader::create(source);
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

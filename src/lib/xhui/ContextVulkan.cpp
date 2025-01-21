#if HAS_LIB_VULKAN

#include "ContextVulkan.h"
#include "Window.h"
#include "Painter.h"
#include "../os/msg.h"
#include "../image/image.h"

namespace xhui {

ContextVulkan::ContextVulkan(Window* w) {
	window = w;

	glfwMakeContextCurrent(w->window);
	api_init();
}

bool ContextVulkan::start() {
	if (!swap_chain->acquire_image(&image_index, image_available_semaphore)) {
		rebuild_default_stuff();
		return false;
	}

	auto f = wait_for_frame_fences[image_index];
	f->wait();
	f->reset();
	return true;
}


void ContextVulkan::_create_swap_chain_and_stuff() {
	swap_chain = vulkan::SwapChain::create_for_glfw(device, window->window);
	auto swap_images = swap_chain->create_textures();
	for (auto t: swap_images)
		wait_for_frame_fences.add(new vulkan::Fence(device));

	for (auto t: swap_images)
		command_buffers.add(device->command_pool->create_command_buffer());

	depth_buffer = swap_chain->create_depth_buffer();
	render_pass = swap_chain->create_render_pass(depth_buffer);
	frame_buffers = swap_chain->create_frame_buffers(render_pass, depth_buffer);
}

void ContextVulkan::api_init() {
	instance = vulkan::init({"glfw", "validation", "api=1.2", "verbosity=2"});
	auto surface = instance->create_glfw_surface(window->window);
	device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation"});
	msg_write("device found");

	//device->create_query_pool(MAX_TIMESTAMP_QUERIES);
	pool = new vulkan::DescriptorPool("buffer:4096,sampler:4096", 65536);


	image_available_semaphore = new vulkan::Semaphore(device);
	render_finished_semaphore = new vulkan::Semaphore(device);


	framebuffer_resized = false;

	_create_swap_chain_and_stuff();

	tex_white = new vulkan::Texture();
	tex_black = new vulkan::Texture();
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));

	vb = new vulkan::VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID, rect::ID);

	//return new Context;

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

		pipeline = new vulkan::GraphicsPipeline(shader, render_pass, 0, "triangles", vb);
		pipeline->set_z(false, false);
		pipeline->set_culling(vulkan::CullMode::NONE);
		pipeline->rebuild();

		pipeline_alpha = new vulkan::GraphicsPipeline(shader, render_pass, 0, "triangles", vb);
		pipeline_alpha->set_z(false, false);
		pipeline_alpha->set_culling(vulkan::CullMode::NONE);
		pipeline_alpha->set_blend(vulkan::Alpha::SOURCE_ALPHA, vulkan::Alpha::SOURCE_INV_ALPHA);
		pipeline_alpha->rebuild();
	} catch (Exception& e) {
		msg_error(e.message());
		throw;
	}


	dset->set_texture(0, tex_white);
	dset->update();
}


void ContextVulkan::rebuild_default_stuff() {
	msg_write("recreate swap chain");

	device->wait_idle();

	//_delete_swap_chain_and_stuff();
	_create_swap_chain_and_stuff();
}

void ContextVulkan::resize(int w, int h) {
	rebuild_default_stuff();
}

vulkan::CommandBuffer* ContextVulkan::current_command_buffer() const {
	return command_buffers[image_index];
}

vulkan::FrameBuffer* ContextVulkan::current_frame_buffer() const {
	return frame_buffers[image_index];
}


}

#endif

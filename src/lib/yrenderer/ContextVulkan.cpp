#include "Context.h"
#include "helper/PipelineManager.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/os/msg.h>
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>
#define HAS_XHUI
#endif


#ifdef USING_VULKAN

namespace yrenderer {

static vulkan::Surface surface; // FIXME

Context::Context(ygfx::Context* ctx) {
	context = ctx;
	device = ctx->device;
}

Context::~Context() {
	delete pool;
	delete context;
}



void _create_context_stuff(Context* ctx) {
	ctx->device->create_query_pool(MAX_TIMESTAMP_QUERIES);
	ctx->pool = new vulkan::DescriptorPool("ubo:65536,sampler:65536", 65536);

	ctx->_create_default_textures();
}

#if HAS_LIB_GLFW
Context* api_init_glfw(GLFWwindow* window) {
	auto instance = vulkan::init({"glfw", "validation", "api=1.3", "rtx?", "verbosity=3"});
	surface = instance->create_glfw_surface(window);
	vulkan::Device* device = nullptr;

	try {
		// try physical first
		device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "multisample", "validation", "physical", "rtx?", "compute?", "meshshader?", "tesselationshader?"});
	} catch (...) {
		msg_error("no valid GPU found. Trying again, allowing CPU backend...");
		device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "multisample", "validation", "physical?", "rtx?", "compute?", "meshshader?", "tesselationshader?"});
	}

	auto ctx = new Context(new ygfx::Context(instance, device));
	_create_context_stuff(ctx);

	return ctx;
}
#endif


Context* api_init_external(ygfx::Context* _ctx) {
	vulkan::default_device = _ctx->device;
	auto ctx = new Context(_ctx);
	_create_context_stuff(ctx);
	return ctx;
}

Context* api_init_xhui(xhui::Painter* p) {
#ifdef HAS_XHUI
	return api_init_external(p->context->context);
#else
	return nullptr;
#endif
}

void api_end(Context* ctx) {
	ctx->gpu_flush();
	PipelineManager::clear();
	delete ctx;
}


void Context::reset_gpu_timestamp_queries() {
	gpu_timestamp_queries.clear();
	gpu_timestamp_queries.simple_reserve(256);
	device->reset_query_pool(0, MAX_TIMESTAMP_QUERIES);
}

void Context::gpu_timestamp(const RenderParams& params, int channel) {
	if (gpu_timestamp_queries.num >= MAX_TIMESTAMP_QUERIES)
		return;
	params.command_buffer->timestamp(gpu_timestamp_queries.num, true);//(channel & (int)0x80000000) != 0);
	gpu_timestamp_queries.add(channel);
}

void Context::gpu_timestamp_begin(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel);
}

void Context::gpu_timestamp_end(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel | (int)0x80000000);
}

Array<float> Context::gpu_read_timestamps() {
	auto tt = device->get_timestamps(0, gpu_timestamp_queries.num);
	Array<float> result;
	result.resize(gpu_timestamp_queries.num);
	for (int i=0; i<gpu_timestamp_queries.num; i++)
		result[i] = (float)(tt[i] - tt[0]) * device->physical_device_properties.limits.timestampPeriod * 1e-9f;
	return result;
}

void Context::gpu_flush() {
	device->wait_idle();
}

}

#endif

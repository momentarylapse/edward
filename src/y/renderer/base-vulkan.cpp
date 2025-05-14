#include "base.h"
#include "helper/PipelineManager.h"
#include "../y/EngineData.h"
#include "../helper/ResourceManager.h"
#include "../graphics-impl.h"
#include <lib/os/msg.h>
#if __has_include(<lib/xhui/Painter.h>)
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>
#define HAS_XHUI
#endif


#ifdef USING_VULKAN

vulkan::Instance *instance = nullptr;
vulkan::DescriptorPool *pool = nullptr;
vulkan::Device *device = nullptr;
vulkan::Surface surface;

Context* _create_context() {
	device->create_query_pool(MAX_TIMESTAMP_QUERIES);
	pool = new vulkan::DescriptorPool("ubo:65536,sampler:65536", 65536);

	_create_default_textures();

	return new Context;
}

#if HAS_LIB_GLFW
Context* api_init(GLFWwindow* window) {
	instance = vulkan::init({"glfw", "validation", "api=1.3", "rtx?", "verbosity=3"});
	surface = instance->create_glfw_surface(window);
	try {
		device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "rtx", "compute"});
		msg_write("device found: RTX + COMPUTE");
	} catch (...) {}

	if (!device) {
		try {
			device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "rtx"});
			msg_write("device found: RTX");
		} catch (...) {}
	}

	if (!device) {
		try {
			device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "compute", "meshshader"});
			msg_write("device found: COMPUTE + MESH SHADER");
		} catch (...) {}
	}

	if (!device) {
		try {
			device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "compute", "tesselationshader"});
			msg_write("device found: COMPUTE + TESSELATION");
		} catch (...) {}
	}

	if (!device) {
		try {
			device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "compute"});
			msg_write("device found: COMPUTE");
		} catch (...) {}
	}

	if (!device) {
		device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation"});
		msg_write("WARNING:  device found: neither RTX nor COMPUTE");
	}

	return _create_context();
}
#endif


Context* api_init_external(vulkan::Instance* _instance, vulkan::Device* _device) {
	instance = _instance;
	device = _device;
	return _create_context();
}

Context* api_init_xhui(xhui::Painter* p) {
#ifdef HAS_XHUI
	vulkan::default_device = p->context->device;
	return api_init_external(p->context->instance, p->context->device);
#else
	return nullptr;
#endif
}

void api_end() {
	gpu_flush();
	PipelineManager::clear();
	engine.resource_manager->clear();
	delete pool;
	delete device;
	delete instance;
}


void reset_gpu_timestamp_queries() {
	gpu_timestamp_queries.clear();
	gpu_timestamp_queries.simple_reserve(256);
	device->reset_query_pool(0, MAX_TIMESTAMP_QUERIES);
}

void gpu_timestamp(const RenderParams& params, int channel) {
	if (gpu_timestamp_queries.num >= MAX_TIMESTAMP_QUERIES)
		return;
	params.command_buffer->timestamp(gpu_timestamp_queries.num, (channel & (int)0x80000000) != 0);
	gpu_timestamp_queries.add(channel);
}

void gpu_timestamp_begin(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel);
}

void gpu_timestamp_end(const RenderParams& params, int channel) {
	gpu_timestamp(params, channel | (int)0x80000000);
}

Array<float> gpu_read_timestamps() {
	auto tt = device->get_timestamps(0, gpu_timestamp_queries.num);
	Array<float> result;
	result.resize(gpu_timestamp_queries.num);
	for (int i=0; i<gpu_timestamp_queries.num; i++)
		result[i] = (float)(tt[i] - tt[0]) * device->physical_device_properties.limits.timestampPeriod * 1e-9f;
	return result;
}

void gpu_flush() {
	device->wait_idle();
}

#endif

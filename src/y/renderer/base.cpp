/*
 * base.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */

#include "base.h"
#include "helper/PipelineManager.h"
#include "../helper/ResourceManager.h"
#include "../graphics-impl.h"
#include "../y/EngineData.h"
#include "../lib/image/image.h"
#include "../lib/os/msg.h"
#include "../Config.h"

Texture *tex_white = nullptr;
Texture *tex_black = nullptr;

static const int MAX_TIMESTAMP_QUERIES = 4096;

Array<int> gpu_timestamp_queries;

#ifdef USING_VULKAN

vulkan::Instance *instance = nullptr;
vulkan::DescriptorPool *pool = nullptr;
vulkan::Device *device = nullptr;
vulkan::Surface surface;

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
			device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation", "compute"});
			msg_write("device found: COMPUTE");
		} catch (...) {}
	}

	if (!device) {
		device = vulkan::Device::create_simple(instance, surface, {"graphics", "present", "swapchain", "anisotropy", "validation"});
		msg_write("WARNING:  device found: neither RTX nor COMPUTE");
	}

	device->create_query_pool(MAX_TIMESTAMP_QUERIES);
	pool = new vulkan::DescriptorPool("buffer:65536,sampler:65536", 65536);

	tex_white = new Texture();
	tex_black = new Texture();
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));

	return new Context;
}
#endif

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

void gpu_timestamp(CommandBuffer *cb, int channel) {
	if (gpu_timestamp_queries.num >= MAX_TIMESTAMP_QUERIES)
		return;
	cb->timestamp(gpu_timestamp_queries.num);
	gpu_timestamp_queries.add(channel);
}

void gpu_timestamp_begin(CommandBuffer *cb, int channel) {
	gpu_timestamp(cb, channel);
}

void gpu_timestamp_end(CommandBuffer *cb, int channel) {
	gpu_timestamp(cb, channel | (int)0x80000000);
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

#ifdef USING_OPENGL


namespace nix {
	extern bool allow_separate_vertex_arrays;
}


Context* api_init(GLFWwindow* window) {
	nix::allow_separate_vertex_arrays = true;
	auto gl = nix::init();

	if (gl->total_mem() > 0) {
		msg_write(format("VRAM: %d mb  of  %d mb available", gl->available_mem() / 1024, gl->total_mem() / 1024));
	}

	nix::create_query_pool(MAX_TIMESTAMP_QUERIES);

	tex_white = new nix::Texture(16, 16, "rgba:i8");
	tex_black = new nix::Texture(16, 16, "rgba:i8");
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));

	return gl;
}

void reset_gpu_timestamp_queries() {
	gpu_timestamp_queries.clear();
	gpu_timestamp_queries.simple_reserve(256);
}

void gpu_timestamp(int channel) {
	if (gpu_timestamp_queries.num >= MAX_TIMESTAMP_QUERIES)
		return;
	nix::query_timestamp(gpu_timestamp_queries.num);
	gpu_timestamp_queries.add(channel);
}

void gpu_timestamp_begin(int channel) {
	gpu_timestamp(channel);
}

void gpu_timestamp_end(int channel) {
	gpu_timestamp(channel | (int)0x80000000);
}

Array<float> gpu_read_timestamps() {
	auto tt = nix::get_timestamps(0, gpu_timestamp_queries.num);
	Array<float> result;
	result.resize(tt.num);
	for (int i=0; i<tt.num; i++)
		result[i] = (float)(tt[i] - tt[0]) * 1e-9f;
	return result;
}

void gpu_flush() {
	nix::flush();
}

void api_end() {
}

#endif



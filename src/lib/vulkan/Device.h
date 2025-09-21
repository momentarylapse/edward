/*
 * Device.h
 *
 *  Created on: Oct 27, 2020
 *      Author: michi
 */

#pragma once

#if HAS_LIB_VULKAN

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/base/set.h>
#include "Queue.h"

namespace vulkan {

	class Instance;
	class CommandPool;
	enum class Feature;


class Device {
public:
	Device();
	~Device();

	Instance *instance = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties physical_device_properties;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR ray_tracing_properties;

	VkSurfaceKHR surface = VK_NULL_HANDLE; // TODO extract

	base::set<Feature> features;
	bool has_rtx() const;
	bool has_compute() const;


	QueueFamilyIndices indices;
	Queue graphics_queue;
	Queue present_queue;
	Queue compute_queue;

	CommandPool *command_pool = nullptr;

	string physical_name() const;

	static xfer<Device> create_simple(Instance *instance, VkSurfaceKHR surface, const Array<string> &op);

	void pick_physical_device(Instance *instance, VkSurfaceKHR surface, const Requirements& req);
	void create_logical_device(VkSurfaceKHR surface);



	void get_rtx_properties();

	uint32_t find_memory_type(const VkMemoryRequirements &requirements, VkMemoryPropertyFlags properties);
	VkFormat find_supported_format(const Array<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat find_depth_format();


	int make_aligned(int size);

	void wait_idle();


	VkQueryPool query_pool;
	void create_query_pool(int count);
	void reset_query_pool(int first, int count);
	Array<int> get_timestamps(int first, int count);
};

extern Device *default_device;

string features_to_string(const base::set<Feature>& features);


} /* namespace vulkan */

#endif

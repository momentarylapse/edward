/*
 * Device.cpp
 *
 *  Created on: Oct 27, 2020
 *      Author: michi
 */

#if HAS_LIB_VULKAN

#include <vulkan/vulkan.h>
#include "Device.h"
#include "vulkan.h"
#include "Queue.h"
#include "helper.h"
#include "common.h"

#include <lib/base/set.h>
#include <lib/base/map.h>
#include <lib/os/msg.h>

namespace vulkan {

Device *default_device;


base::map<Feature, string> feature_names() {
	return {{
		{Feature::VALIDATION, "validation"},
		{Feature::GRAPHICS, "graphics"},
		{Feature::PRESENT, "present"},
		{Feature::COMPUTE, "compute"},
		{Feature::SWAP_CHAIN, "swapchain"},
		{Feature::ANISOTROPY, "anisotropy"},
		{Feature::GEOMETRY_SHADER, "geometryshader"},
		{Feature::TESSELATION_SHADER, "tesselationshader"},
		{Feature::RTX, "rtx"},
		{Feature::MESH_SHADER, "meshshader"},
		{Feature::MULTISAMPLE, "multisample"},
		{Feature::PHYSICAL, "physical"},
	}};
}

	bool check_device_extension_support(VkPhysicalDevice device, const Requirements& req);
	Array<VkQueueFamilyProperties> get_queue_families(VkPhysicalDevice device);
	string result2str(VkResult r);



	Array<const char*> device_feature_extensions(Feature feature) {
		if (feature == Feature::SWAP_CHAIN)
			return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		if (feature == Feature::RTX)
			return {
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};
		if (feature == Feature::MESH_SHADER)
			return {"VK_NV_mesh_shader"};
			//ext.add("VK_EXT_mesh_shader"); // VK_EXT_MESH_SHADER_EXTENSION_NAME
		return {};
	}

	//extern Array<const char*> validation_layers;

base::set<string> device_get_available_extensions(VkPhysicalDevice device) {
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

	Array<VkExtensionProperties> extensions;
	extensions.resize((int)extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, &extensions[0]);

	base::set<string> names;
	for (const auto& ext: extensions)
		names.add(ext.extensionName);
	return names;
}

base::set<Feature> device_features(VkPhysicalDevice device, VkSurfaceKHR surface) {
	base::set<Feature> features;

	features.add(Feature::VALIDATION);

	const auto indices = QueueFamilyIndices::query(device, surface);
	if (indices.graphics_family)
		features.add(Feature::GRAPHICS);
	if (indices.compute_family)
		features.add(Feature::COMPUTE);

	const auto available_extensions = device_get_available_extensions(device);


	//	if (verbosity >= 3)
	//		msg_write("---- GPU-----");
	//	for (const auto& extension : available_extensions)
	//		msg_write("  available:   " + string(extension));
	//	for (const auto& extension : required_extensions)
	//		msg_write("  required:   " + extension);

	auto check_extensions = [&available_extensions](Feature feature) {
		auto required_extensions = device_feature_extensions(feature);
		for (auto e: required_extensions)
			if (!available_extensions.contains(e))
				return false;
		return true;
	};

	if (check_extensions(Feature::RTX))
		features.add(Feature::RTX);
	if (check_extensions(Feature::MESH_SHADER))
		features.add(Feature::MESH_SHADER);

	VkPhysicalDeviceFeatures supported_features;
	vkGetPhysicalDeviceFeatures(device, &supported_features);
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);
	//msg_write(properties.deviceName);

	if (surface) {
		SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device, surface);
		if (swapChainSupport.formats.num > 0 and check_extensions(Feature::SWAP_CHAIN))
			features.add(Feature::SWAP_CHAIN);
		if (swapChainSupport.present_modes.num > 0 and indices.present_family)
			features.add(Feature::PRESENT);
	} else {
		if (indices.present_family)
			features.add(Feature::PRESENT);
	}

	if (supported_features.samplerAnisotropy)
		features.add(Feature::ANISOTROPY);
	if (supported_features.geometryShader)
		features.add(Feature::GEOMETRY_SHADER);
	if (supported_features.tessellationShader)
		features.add(Feature::TESSELATION_SHADER);

	if (properties.limits.framebufferColorSampleCounts > 1)
		features.add(Feature::MULTISAMPLE);

	if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_CPU)
		features.add(Feature::PHYSICAL); // simulated gpu :/
	return features;
}

int device_suitable_rating(VkPhysicalDevice device, VkSurfaceKHR surface, const Requirements& req) {
	auto features = device_features(device, surface);

	/*VkPhysicalDeviceProperties p;
	vkGetPhysicalDeviceProperties(device, &p);
	msg_write("  PHYSICAL: " + string(p.deviceName));*/

	for (auto f: req.required)
		if (!features.contains(f)) {
			//msg_write("MISSING: " + feature_names()[f]);
			return -1;
		}

	int rating = 1;
	for (auto f: req.optional)
		if (features.contains(f))
			rating ++;
	//msg_write(rating);
	return rating;
}

bool check_device_extension_support(VkPhysicalDevice device, const Requirements& req) {
	const auto available_extensions = device_get_available_extensions(device);

	//	if (verbosity >= 3)
	//		msg_write("---- GPU-----");
	//	for (const auto& extension : available_extensions)
	//		msg_write("  available:   " + string(extension.extensionName));
	//	for (const auto& extension : required_extensions)
	//		msg_write("  required:   " + extension);

	for (const auto f: req.required) {
		auto required_extensions = device_feature_extensions(f);
		for (auto e: required_extensions)
			if (!available_extensions.find(e))
				return false;
	}

	return true;
}


Device::Device() {
}

Device::~Device() {
	if (command_pool)
		delete command_pool;
	if (surface)
		vkDestroySurfaceKHR(instance->instance, surface, nullptr);
	if (device)
		vkDestroyDevice(device, nullptr);
}

string features_to_string(const base::set<Feature>& features) {
	Array<string> names;
	for (auto f: features)
		names.add(feature_names()[f]);
	return implode(names, ", ");
}

void show_physical_devices(const Array<VkPhysicalDevice>& devices, VkSurfaceKHR surface) {
	msg_write(format("%d devices found:", devices.num));
	for (const auto& dev: devices) {
		VkPhysicalDeviceProperties p;
		vkGetPhysicalDeviceProperties(dev, &p);
		msg_write("  PHYSICAL: " + string(p.deviceName));
		msg_write("     > " + features_to_string(device_features(dev, surface)));
	}
}

Array<VkPhysicalDevice> get_all_physical_devices(Instance *instance) {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance->instance, &device_count, nullptr);

	if (device_count == 0)
		throw Exception("failed to find GPUs with Vulkan support!");

	Array<VkPhysicalDevice> devices;
	devices.resize(device_count);
	vkEnumeratePhysicalDevices(instance->instance, &device_count, &devices[0]);
	return devices;
}

string Device::physical_name() const {
	return physical_device_properties.deviceName;
}

void Device::pick_physical_device(Instance *_instance, VkSurfaceKHR _surface, const Requirements& req) {
	instance = _instance;
	surface = _surface;

	auto devices = get_all_physical_devices(instance);

	if (verbosity >= 3)
		show_physical_devices(devices, surface);

	physical_device = VK_NULL_HANDLE;
	int max_rating = 0;
	for (const auto& dev: devices) {
		int rating = device_suitable_rating(dev, surface, req);
		if (rating > max_rating) {
			physical_device = dev;
			max_rating = rating;
		}
	}

	if (physical_device == VK_NULL_HANDLE)
		throw Exception("failed to find a suitable GPU!");

	features.clear();
	for (auto f: device_features(physical_device, surface))
		if (req.required.contains(f) or req.optional.contains(f))
			features.add(f);

	vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
	if (verbosity >= 1) {
		msg_write("device chosen: " + physical_name());
		msg_write("features: " + features_to_string(features));
	}
	if (verbosity >= 3 and false) {
		msg_write(" props:");
		msg_write("  minUniformBufferOffsetAlignment  " + i2s(physical_device_properties.limits.minUniformBufferOffsetAlignment));
		msg_write("  maxPushConstantsSize  " + i2s(physical_device_properties.limits.maxPushConstantsSize));
		msg_write("  maxImageDimension2D  " + i2s(physical_device_properties.limits.maxImageDimension2D));
		msg_write("  maxUniformBufferRange  " + i2s(physical_device_properties.limits.maxUniformBufferRange));
		msg_write("  maxPerStageDescriptorUniformBuffers  " + i2s(physical_device_properties.limits.maxPerStageDescriptorUniformBuffers));
		msg_write("  maxPerStageDescriptorSamplers  " + i2s(physical_device_properties.limits.maxPerStageDescriptorSamplers));
		msg_write("  maxDdevice->escriptorSetSamplers  " + i2s(physical_device_properties.limits.maxDescriptorSetSamplers));
		msg_write("  maxDescriptorSetUniformBuffers  " + i2s(physical_device_properties.limits.maxDescriptorSetUniformBuffers));
		msg_write("  maxDescriptorSetUniformBuffersDynamic  " + i2s(physical_device_properties.limits.maxDescriptorSetUniformBuffersDynamic));
		//std::cout << "  maxDescriptorSetUniformBuffers  " << physical_device_properties.limits.maxDescriptorSetUniformBuffers << "\n";
		//std::cout << "  maxDescriptorSetUniformBuffers  " << physical_device_properties.limits.maxDescriptorSetUniformBuffers << "\n";
	}

	/*VkPhysicalDeviceRayTracingFeaturesKHR rtf = {};

	VkPhysicalDeviceFeatures2 dp2 = {};
	dp2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	vkGetPhysicalDeviceFeatures2(physical_device, &dp2);*/
	if (verbosity >= 2)
		msg_write(" done");
}

void Device::create_logical_device(VkSurfaceKHR surface) {
	//features = req.required; // TODO
	indices = QueueFamilyIndices::query(physical_device, surface);

	Array<VkDeviceQueueCreateInfo> queue_create_infos;
	auto unique_queue_families = indices.unique();

	float queue_priority = 1.0f;
	for (uint32_t queue_family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.add(queue_create_info);
	}

	VkPhysicalDeviceVulkan12Features features12 = {};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.hostQueryReset = VK_TRUE;
	features12.bufferDeviceAddress = VK_TRUE;
	features12.scalarBlockLayout = VK_TRUE;

	VkPhysicalDeviceFeatures2 device_features = {};
	device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	device_features.pNext = &features12;

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt_features = {};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features = {};
	if (features.contains(Feature::GEOMETRY_SHADER))
		device_features.features.geometryShader = VK_TRUE;
	if (features.contains(Feature::TESSELATION_SHADER))
		device_features.features.tessellationShader = VK_TRUE;
	if (features.contains(Feature::ANISOTROPY))
		device_features.features.samplerAnisotropy = VK_TRUE;
	if (features.contains(Feature::MULTISAMPLE))
		device_features.features.sampleRateShading = VK_TRUE;
	if (features.contains(Feature::RTX)) {
		rt_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rt_features.rayTracingPipeline = VK_TRUE;

		as_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		as_features.accelerationStructure = VK_TRUE;
		as_features.pNext = &rt_features;

		features12.pNext = &as_features;
	}

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = &device_features;

	create_info.queueCreateInfoCount = queue_create_infos.num;
	create_info.pQueueCreateInfos = &queue_create_infos[0];


	Array<const char*> extensions;
	for (auto f: features)
		extensions.append(device_feature_extensions(f));
#ifdef OS_MAC
	extensions.add("VK_KHR_portability_subset");
#endif
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.num);
	create_info.ppEnabledExtensionNames = &extensions[0];

	/*if (req & Requirements::VALIDATION) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.num);
		create_info.ppEnabledLayerNames = &validation_layers[0];
	} else {
		create_info.enabledLayerCount = 0;
	}*/

	auto r = vkCreateDevice(physical_device, &create_info, nullptr, &device);
	if (r != VK_SUCCESS)
		throw Exception("failed to create logical device!  " + result2str(r));

	if (indices.graphics_family)
		vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue.queue);
	if (indices.present_family)
		vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue.queue);
	if (indices.compute_family)
		vkGetDeviceQueue(device, indices.compute_family.value(), 0, &compute_queue.queue);
}



void Device::wait_idle() {
	vkDeviceWaitIdle(device);
}



uint32_t Device::find_memory_type(const VkMemoryRequirements &requirements, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	for (uint32_t i=0; i<memProperties.memoryTypeCount; i++) {
		if ((requirements.memoryTypeBits & (1 << i)) and (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw Exception("failed to find suitable memory type!");
}


VkFormat Device::find_supported_format(const Array<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format: candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR and (props.linearTilingFeatures & features) == features)
			return format;
		if (tiling == VK_IMAGE_TILING_OPTIMAL and (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw Exception("failed to find supported format!");
}

VkFormat Device::find_depth_format() {
	return find_supported_format(
	{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}




bool Device::has_rtx() const {
	return features.contains(Feature::RTX);
}

bool Device::has_compute() const {
	return features.contains(Feature::COMPUTE);
}

void Device::create_query_pool(int count) {
	VkQueryPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	info.queryType = VK_QUERY_TYPE_TIMESTAMP;
	info.queryCount = count;
	vkCreateQueryPool(device, &info, nullptr, &query_pool);
}

void Device::reset_query_pool(int first, int count) {
	vkResetQueryPool(device, query_pool, first, count);
}

Array<int> Device::get_timestamps(int first, int count) {
	Array<int> tt;
	tt.resize(count);
	vkGetQueryPoolResults(device, query_pool, first, count, sizeof(tt[0]) * tt.num, &tt[0], 4, VK_QUERY_RESULT_PARTIAL_BIT);//VK_QUERY_RESULT_WAIT_BIT);
	return tt;
}


int Device::make_aligned(int size) {
	if (physical_device_properties.limits.minUniformBufferOffsetAlignment == 0)
		return 0;
	return (size + physical_device_properties.limits.minUniformBufferOffsetAlignment - 1) & ~(size - 1);
}


Requirements parse_requirements(const Array<string> &op) {
	Requirements req;
	const auto map = feature_names();
	auto parse = [&map] (const string& s) -> Feature {
		for (const auto& [f, name]: map)
			if (name == s)
				return f;
		throw Exception("unknown required feature: " + s);
	};

	for (auto &o: op) {
		if (o.tail(1) == "?")
			req.optional.add(parse(o.sub_ref(0, -1)));
		else
			req.required.add(parse(o));
	}
	return req;
}

xfer<Device> Device::create_simple(Instance *instance, VkSurfaceKHR surface, const Array<string> &op) {
	//op.append({"graphics", "present", "swapchain", "anisotropy"});
	auto req = parse_requirements(op);
	auto device = new Device();
	device->pick_physical_device(instance, surface, req);
	device->create_logical_device(surface);

	device->command_pool = new CommandPool(device);

	if (sa_contains(op, "rtx"))
		device->get_rtx_properties();

	default_device = device;
	return device;
}



void Device::get_rtx_properties() {

	ray_tracing_properties = {};
	ray_tracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	VkPhysicalDeviceProperties2 dev_props = {};
	dev_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	dev_props.pNext = &ray_tracing_properties;
	dev_props.properties = {};

	//pvkGetPhysicalDeviceProperties2() FIXME
	_vkGetPhysicalDeviceProperties2(physical_device, &dev_props);
	if (verbosity >= 3) {
		msg_write("PROPS");
		msg_write(format("shader group stride: ", ray_tracing_properties.maxShaderGroupStride));
		msg_write(format("shader group base alignment: ", ray_tracing_properties.shaderGroupBaseAlignment));
		msg_write(format("shader group handle size: ", ray_tracing_properties.shaderGroupHandleSize));
	}
}



} /* namespace vulkan */

#endif

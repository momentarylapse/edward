/*
 * Queue.cpp
 *
 *  Created on: Sep 21, 2021
 *      Author: michi
 */

#if HAS_LIB_VULKAN

#include <vulkan/vulkan.h>
#include "Queue.h"
#include "Semaphore.h"
#include "vulkan.h"
#include "helper.h"
#include "common.h"
#include "../base/set.h"
#include "../base/iter.h"

namespace vulkan {


bool Queue::submit(CommandBuffer *cb, const Array<Semaphore*> &wait_sem, const Array<Semaphore*> &signal_sem, Fence *fence) {

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	auto wait_semaphores = extract_semaphores(wait_sem);
	auto signal_semaphores = extract_semaphores(signal_sem);
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = wait_semaphores.num;
	submit_info.pWaitSemaphores = &wait_semaphores[0];
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cb->buffer;
	submit_info.signalSemaphoreCount = signal_semaphores.num;
	submit_info.pSignalSemaphores = &signal_semaphores[0];


	if (fence)
		fence->reset();

	VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence_handle(fence));
	return (result == VK_SUCCESS);
	/*if (result != VK_SUCCESS) {
		std::cerr << " SUBMIT ERROR " << result << "\n";
		throw Exception("failed to submit draw command buffer!");
	}*/
}

void Queue::wait_idle() {
	vkQueueWaitIdle(queue);
}


Array<VkQueueFamilyProperties> get_queue_families(VkPhysicalDevice device) {
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	Array<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, &queue_families[0]);
	return queue_families;
}


QueueFamilyIndices QueueFamilyIndices::query(VkPhysicalDevice device, VkSurfaceKHR surface) {
	auto queue_families = get_queue_families(device);

	QueueFamilyIndices indices;
	for (auto&& [i,family]: enumerate(queue_families)) {
		if (family.queueCount == 0)
			continue;

		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT and !indices.graphics_family)
			indices.graphics_family = i;

		if (family.queueFlags & VK_QUEUE_COMPUTE_BIT and !indices.compute_family)
			indices.compute_family = i;

		if (surface) {
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
			if (present_support and !indices.present_family)
				indices.present_family = i;
		}
	}
	return indices;
}
bool QueueFamilyIndices::is_complete(const Requirements& req) const {
	if (req.required.contains(Feature::GRAPHICS) and !graphics_family)
		return false;
	if (req.required.contains(Feature::COMPUTE) and !compute_family)
		return false;
	if (req.required.contains(Feature::PRESENT) and !present_family)
		return false;
	return true;
}

Array<uint32_t> QueueFamilyIndices::unique() const {
	base::set<uint32_t> unique_queue_families;
	if (graphics_family)
		unique_queue_families.add(*graphics_family);
	if (present_family)
		unique_queue_families.add(*present_family);
	if (compute_family)
		unique_queue_families.add(*compute_family);
	return unique_queue_families;
}


/*bool QueueFamilyIndices::is_complete() {
	return graphics_family.has_value() and present_family.has_value();
}*/



} /* namespace vulkan */

#endif

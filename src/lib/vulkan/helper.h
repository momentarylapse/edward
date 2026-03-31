#pragma once

#if HAS_LIB_VULKAN

#include "../base/base.h"
#include <vulkan/vulkan.h>

namespace vulkan{

	class FrameBuffer;
	class CommandBuffer;

	struct ImageAndMemory {
		VkImage image = nullptr;
		VkDeviceMemory memory = nullptr;
		VkFormat format = VK_FORMAT_UNDEFINED;

		void create(VkImageType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels, uint32_t num_layers, VkSampleCountFlagBits samples, VkFormat format, VkImageUsageFlags usage, bool cube);
		void _destroy();


		void generate_mipmaps(CommandBuffer* cb, uint32_t width, uint32_t height, uint32_t mip_levels, uint32_t layer0, uint32_t num_layers, VkImageLayout new_layout);

		VkImageView create_view(VkImageAspectFlags aspect_flags, VkImageViewType type, uint32_t mip_levels, uint32_t layer0, uint32_t num_layers) const;
		void transition_layout(CommandBuffer* cb, VkImageLayout old_layout, VkAccessFlags source_flags, VkPipelineStageFlags source_stage,
			VkImageLayout new_layout, VkAccessFlags dest_flags, VkPipelineStageFlags dest_stage,
			uint32_t mip_levels, uint32_t layer0, uint32_t num_layers) const;


		bool is_depth_buffer() const;
		bool has_stencil_component() const;
		VkImageAspectFlagBits aspect() const;
	};

	//void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
	void copy_buffer(CommandBuffer* cb, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
	void copy_buffer_to_image(CommandBuffer* cb, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, uint32_t level, uint32_t layer);
	void copy_image_to_buffer(CommandBuffer* cb, VkImage image, uint32_t width, uint32_t height, uint32_t depth, uint32_t level, uint32_t layer, VkBuffer buffer);
};

#endif

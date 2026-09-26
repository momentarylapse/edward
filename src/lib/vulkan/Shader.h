#pragma once

#if HAS_LIB_VULKAN



#include "../base/base.h"
#include "../base/error.h"
#include "../base/pointer.h"
#include "../os/path.h"
#include <vulkan/vulkan.h>
#include "helper.h"

namespace vulkan{

	class Shader : public Sharable<base::Empty> {
	public:
		Shader();
		~Shader();

		struct Module {
			VkShaderModule module;
			VkShaderStageFlagBits stage;
		};
		Array<Module> modules;

		Array<VkDescriptorSetLayout> descr_layouts;
		int push_size;

		void set_meta(const string& bindings, int push_size);
		base::result_void add_module(const string& stage, const string& source);

		VkShaderModule get_module(VkShaderStageFlagBits stage) const;
	};

};

#endif

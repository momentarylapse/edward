#include "vulkan.h"
#include "../kabaexport/KabaExporter.h"

#ifdef HAS_LIB_VULKAN

#define vul_p(p)		p




KABA_LINK_GROUP_BEGIN

xfer<vulkan::Texture> __vulkan_load_texture(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Texture::load(filename));
	return nullptr;
}

xfer<vulkan::Shader> __vulkan_load_shader(const Path &filename) {
	KABA_EXCEPTION_WRAPPER(return vulkan::Shader::load(filename));
	return nullptr;
}

xfer<void> __vulkan_init(const Array<string> &op) {
	KABA_EXCEPTION_WRAPPER(return vulkan::init(op));
	return nullptr;
}

xfer<vulkan::Device> __vulkan_device_create_simple(vulkan::Instance *instance, void* surface, const Array<string> &op) {
#if HAS_LIB_GLFW
	KABA_EXCEPTION_WRAPPER(return vulkan::Device::create_simple(instance, (vulkan::Surface)surface, op));
#endif
	return nullptr;
}

KABA_LINK_GROUP_END

class VulkanTexture : vulkan::Texture {
public:
	void __init__() {
		new(this) vulkan::Texture();
	}
	void __init_ext__(int w, int h, const string &format) {
		new(this) vulkan::Texture(w, h, format);
	}
	void __delete__() {
		this->~VulkanTexture();
	}
};

class VulkanVolumeTexture : public vulkan::VolumeTexture {
public:
	void __init__(int nx, int ny, int nz, const string &format) {
		new(this) vulkan::VolumeTexture(nx, ny, nz, format);
	}
};

class VulkanStorageTexture : public vulkan::StorageTexture {
public:
	void __init__(int nx, int ny, int nz, const string &format) {
		new(this) vulkan::StorageTexture(nx, ny, nz, format);
	}
};

class VulkanCubeMap : public vulkan::CubeMap {
public:
	void __init__(int size, const string &format) {
		new(this) vulkan::CubeMap(size, format);
	}
};

class VulkanVertexList : public Array<vulkan::Vertex1> {
public:
	void __init__() {
		new(this) VulkanVertexList;
	}
};

class VulkanDepthBuffer : public vulkan::DepthBuffer {
public:
	void __init__(int w, int h, const string &format, bool with_sampler) {
		new(this) vulkan::DepthBuffer(w, h, format, with_sampler);
	}
};

class VulkanFrameBuffer : public vulkan::FrameBuffer {
public:
	void __init__(vulkan::RenderPass *rp, const shared_array<vulkan::Texture> &attachments) {
		new(this) vulkan::FrameBuffer(rp, attachments);
	}
	void __delete__() {
		this->~VulkanFrameBuffer();
	}
};

class VulkanCommandBuffer : public vulkan::CommandBuffer {
public:
	void __delete__() {
		this->~VulkanCommandBuffer();
	}
};

class VulkanUniformBuffer : public vulkan::UniformBuffer {
public:
	void __init__(int size) {
		new(this) vulkan::UniformBuffer(size);
	}
	void __init_multi__(int size, int count) {
		new(this) vulkan::UniformBuffer(size, count);
	}
	void __delete__() {
		this->~VulkanUniformBuffer();
	}
};

class VulkanDescriptorPool : public vulkan::DescriptorPool {
public:
	void __init__(const string &s, int max_sets) {
		new(this) DescriptorPool(s, max_sets);
	}
	void __delete__() {
		this->~VulkanDescriptorPool();
	}
};

class VulkanDescriptorSet : public vulkan::DescriptorSet {
public:
	void __delete__() {
		this->~VulkanDescriptorSet();
	}
};

class VulkanInstance : public vulkan::Instance {
public:
	void __delete__() {
		this->~VulkanInstance();
	}
	vulkan::Surface _create_glfw_surface(void* window) {
#ifdef HAS_LIB_GLFW
		KABA_EXCEPTION_WRAPPER(return create_glfw_surface((GLFWwindow*)window));
#endif
		return nullptr;
	}
	vulkan::Surface _create_headless_surface() {
		KABA_EXCEPTION_WRAPPER(return create_headless_surface());
		return nullptr;
	}
};

class VulkanVertexBuffer : public vulkan::VertexBuffer {
public:
	void __init__(const string &format) {
		new(this) vulkan::VertexBuffer(format);
	}
	void __delete__() {
		this->~VulkanVertexBuffer();
	}
};

class VulkanGraphicsPipeline : public vulkan::GraphicsPipeline {
public:
	void __init__(vulkan::Shader *shader, vulkan::RenderPass *render_pass, int subpass, const string &topology, const string &format) {
		new(this) vulkan::GraphicsPipeline(shader, render_pass, subpass, vulkan::parse_topology(topology), format);
	}
	void __delete__() {
		this->~VulkanGraphicsPipeline();
	}
};

class VulkanComputePipeline : public vulkan::ComputePipeline {
public:
	void __init__(vulkan::Shader *shaders) {
		new(this) vulkan::ComputePipeline(shaders);
	}
};

class VulkanRayPipeline : public vulkan::RayPipeline {
public:
	void __init__(const string &dset_layouts, const Array<vulkan::Shader*> &shaders, int recursion_depth) {
		new (this) vulkan::RayPipeline(dset_layouts, shaders, recursion_depth);
	}
};

class VulkanRenderPass : public vulkan::RenderPass {
public:
	void __init__(const Array<string> &formats, const Array<string> &options) {
		new(this) vulkan::RenderPass(formats, options);
	}
	void __delete__() {
		this->~VulkanRenderPass();
	}
};

class VulkanSwapChain : public vulkan::SwapChain {
public:
	void __delete__() {
		this->~VulkanSwapChain();
	}
	base::optional<int> acquire_image_x(vulkan::Semaphore *sem) {
		int index;
		if (vulkan::SwapChain::acquire_image(&index, sem))
			return index;
		return base::None;
	}
	static xfer<vulkan::SwapChain> _create_for_glfw(vulkan::Device* device, void* window) {
#ifdef HAS_LIB_GLFW
		KABA_EXCEPTION_WRAPPER(return create_for_glfw(device, (GLFWwindow*)window));
#endif
		return nullptr;
	}
	static xfer<vulkan::SwapChain> _create(vulkan::Device* device, int width, int height) {
		KABA_EXCEPTION_WRAPPER(return create(device, width, height));
		return nullptr;
	}
};

class VulkanFence : public vulkan::Fence {
public:
	void __init__(vulkan::Device *device) {
		new(this) vulkan::Fence(device);
	}
	void __delete__() {
		this->~VulkanFence();
	}
};

class VulkanSemaphore : public vulkan::Semaphore {
public:
	void __init__(vulkan::Device *device) {
		new(this) vulkan::Semaphore(device);
	}
	void __delete__() {
		this->~VulkanSemaphore();
	}
};


class VulkanShader : public vulkan::Shader {
public:
	void __init__() {
		new(this) vulkan::Shader();
	}
	void __delete__() {
		this->~VulkanShader();
	}
};

class VulkanVertex : public vulkan::Vertex1 {
public:
	void __assign__(const VulkanVertex &o) { *this = o; }
};


class ColorList : public Array<color> {
public:
	void __init__() {
		new(this) ColorList;
	}
	void __delete__() {
		this->~ColorList();
	}
	void __assign__(ColorList &o) {
		*this = o;
	}
};




void export_package_vulkan(kaba::Exporter* e) {
	//printf("<vulkan export>\n");
	e->declare_class_size("Instance", sizeof(vulkan::Instance));
	e->link_class_func("Instance.__delete__", &kaba::generic_delete<vulkan::Instance>);
	e->link_class_func("Instance.create_glfw_surface", &VulkanInstance::_create_glfw_surface);
	e->link_class_func("create_headless_surface", &VulkanInstance::_create_headless_surface);
}


#else

void export_package_vulkan(kaba::Exporter* e) {
}

#endif



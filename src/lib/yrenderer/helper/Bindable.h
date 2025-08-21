//
// Created by michi on 1/5/25.
//

#ifndef BINDABLE_H
#define BINDABLE_H

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/any/any.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace yrenderer {

struct RenderParams;

struct Binding {
	enum class Type {
		Texture,
		Image,
		UniformBuffer,
		StorageBuffer
	};
	int index;
	Type type;
	void* p;
};

struct BindingData {
	explicit BindingData(ygfx::Shader* shader);
	Any shader_data; // must be offset=0 for kaba interface...
	Array<Binding> bindings;

	void bind_texture(int index, ygfx::Texture* texture);
	void bind_textures(int index0, const Array<ygfx::Texture*>& textures);
	void bind_image(int index, ygfx::ImageTexture* image);
	void bind_uniform_buffer(int index, ygfx::Buffer* buffer);
	void bind_storage_buffer(int index, ygfx::Buffer* buffer);

	void apply(ygfx::Shader* shader, const RenderParams& params);

#ifdef USING_VULKAN
	owned<vulkan::DescriptorPool> pool;
	owned<vulkan::DescriptorSet> dset;
#endif
};

void apply_shader_data(const RenderParams& params, ygfx::Shader* s, const Any& shader_data);

}


// sorry, but template<T> class Bindable : T... still sucks. Especially the constructor
// (and multi-inheritance clashes with kaba)
#define IMPLEMENT_BINDABLE_INTERFACE \
	BindingData bindings; \
	void bind_texture(int index, ygfx::Texture* texture) { \
		bindings.bind_texture(index, texture); \
	} \
	void bind_textures(int index0, const Array<ygfx::Texture*>& textures) { \
		bindings.bind_textures(index0, textures); \
	} \
	void bind_image(int index, ygfx::ImageTexture* image) { \
		bindings.bind_image(index, image); \
	} \
	void bind_uniform_buffer(int index, ygfx::Buffer* buffer) { \
		bindings.bind_uniform_buffer(index, buffer); \
	} \
	void bind_storage_buffer(int index, ygfx::Buffer* buffer) { \
		bindings.bind_storage_buffer(index, buffer); \
	} \


#endif //BINDABLE_H

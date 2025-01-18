//
// Created by michi on 1/5/25.
//

#ifndef BINDABLE_H
#define BINDABLE_H

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/any/any.h>
#include "../../graphics-fwd.h"

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
	explicit BindingData(Shader* shader);
	Any shader_data; // must be offset=0 for kaba interface...
	Array<Binding> bindings;

	void bind_texture(int index, Texture* texture);
	void bind_textures(int index0, const Array<Texture*>& textures);
	void bind_image(int index, ImageTexture* image);
	void bind_uniform_buffer(int index, Buffer* buffer);
	void bind_storage_buffer(int index, Buffer* buffer);

	void apply(Shader* shader, const RenderParams& params);

#ifdef USING_VULKAN
	owned<vulkan::DescriptorPool> pool;
	owned<vulkan::DescriptorSet> dset;
#endif
};


// sorry, but template<T> class Bindable : T... still sucks. Especially the constructor
// (and multi-inheritance clashes with kaba)
#define IMPLEMENT_BINDABLE_INTERFACE \
	BindingData bindings; \
	void bind_texture(int index, Texture* texture) { \
		bindings.bind_texture(index, texture); \
	} \
	void bind_textures(int index0, const Array<Texture*>& textures) { \
		bindings.bind_textures(index0, textures); \
	} \
	void bind_image(int index, ImageTexture* image) { \
		bindings.bind_image(index, image); \
	} \
	void bind_uniform_buffer(int index, Buffer* buffer) { \
		bindings.bind_uniform_buffer(index, buffer); \
	} \
	void bind_storage_buffer(int index, Buffer* buffer) { \
		bindings.bind_storage_buffer(index, buffer); \
	} \


class mat4;
class vec2;
class vec3;

Any mat4_to_any(const mat4& m);
Any vec2_to_any(const vec2& v);
Any vec3_to_any(const vec3& v);


#endif //BINDABLE_H

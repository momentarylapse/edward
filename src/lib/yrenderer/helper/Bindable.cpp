//
// Created by michi on 1/5/25.
//

#include "Bindable.h"
#include "../Renderer.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/iter.h>
#include <lib/os/msg.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>


Any mat4_to_any(const mat4& m) {
	Any a = Any::EmptyList;
	for (int i=0; i<16; i++)
		a.list_set(i, ((float*)&m)[i]);
	return a;
}
Any vec2_to_any(const vec2& v) {
	Any a = Any::EmptyList;
	a.list_set(0, v.x);
	a.list_set(1, v.y);
	return a;
}
Any vec3_to_any(const vec3& v) {
	Any a = Any::EmptyList;
	a.list_set(0, v.x);
	a.list_set(1, v.y);
	a.list_set(2, v.z);
	return a;
}

using namespace ygfx;

namespace yrenderer {

#ifdef USING_VULKAN
void apply_shader_data(CommandBuffer* cb, const Any &shader_data);
#else
void apply_shader_data(Shader *s, const Any &shader_data);
#endif


#ifdef USING_VULKAN
void apply_shader_data(CommandBuffer* cb, const Any &shader_data) {
	if (shader_data.is_empty()) {
		return;
	} else if (shader_data.is_dict()) {
		char temp[256];
		int max_used = 0;
		for (auto &key: shader_data.keys()) {
			auto &val = shader_data[key];
			auto x = key.explode(":");
			if (x.num < 2) {
				msg_write("invalid shader data key (:offset missing): " + key);
				continue;
			}
			string name = x[0];
			int offset = x[1]._int();
			if (val.is_float()) {
				*(float*)&temp[offset] = val.as_float();
				max_used = max(max_used, offset + 4);
			} else if (val.is_int()) {
				*(int*)&temp[offset] = val.as_int();
				max_used = max(max_used, offset + 4);
			} else if (val.is_bool()) {
				*(bool*)&temp[offset] = val.as_bool();
				max_used = max(max_used, offset + 1);
			} else if (val.is_list()) {
				for (int i=0; i<val.as_list().num; i++)
					*(float*)&temp[offset + i * 4] = val.as_list()[i].as_float();
				max_used = max(max_used, offset + 4 * val.as_list().num);
			} else {
				msg_write("invalid shader data item: " + val.str());
			}
		}
	//	msg_write(bytes(&temp, max_used).hex());
		cb->push_constant(0, max_used, &temp);
	} else {
		msg_write("invalid shader data: " + shader_data.str());
	}
}
#else
void apply_shader_data(Shader *s, const Any &shader_data) {
	if (shader_data.is_empty()) {
		return;
	} else if (shader_data.is_dict()) {
		for (auto &key: shader_data.keys()) {
			auto &val = shader_data[key];
			string name = key.explode(":")[0];
			if (val.is_float()) {
				s->set_float(name, val.as_float());
			} else if (val.is_int()) {
				s->set_int(name, val.as_int());
			} else if (val.is_bool()) {
				s->set_int(name, (int)val.as_bool());
			} else if (val.is_list()) {
				float ff[128];
				for (int i=0; i<val.as_list().num; i++)
					ff[i] = val.as_list()[i].as_float();
				s->set_floats(name, ff, val.as_list().num);
			} else {
				msg_write("invalid shader data item: " + val.str());
			}
		}
	} else {
		msg_write("invalid shader data: " + shader_data.str());
	}
}
#endif


BindingData::BindingData(Shader* shader) {
#ifdef USING_VULKAN
	if (shader) {
		pool = new vulkan::DescriptorPool("sampler:8,buffer:8,storage-buffer:8,image:8", 1);
		dset = pool->create_set_from_layout(shader->descr_layouts[0]);
	}
#endif
}



void BindingData::bind_texture(int index, Texture *texture) {
#ifdef USING_OPENGL
	bindings.add({index, Binding::Type::Texture, texture});
#endif
#ifdef USING_VULKAN
	dset->set_texture(index, texture);
	dset->update();
#endif
}

void BindingData::bind_textures(int index0, const Array<Texture*>& textures) {
	for (auto&& [i, t]: enumerate(textures))
		bind_texture(index0 + i, t);
}

void BindingData::bind_image(int index, ImageTexture *texture) {
#ifdef USING_OPENGL
	bindings.add({index, Binding::Type::Image, texture});
#endif
#ifdef USING_VULKAN
	dset->set_storage_image(index, texture);
	dset->update();
#endif
}

void BindingData::bind_uniform_buffer(int index, Buffer *buffer) {
#ifdef USING_OPENGL
	bindings.add({index, Binding::Type::UniformBuffer, buffer});
#endif
#ifdef USING_VULKAN
	dset->set_uniform_buffer(index, buffer);
	dset->update();
#endif
}

void BindingData::bind_storage_buffer(int index, Buffer *buffer) {
#ifdef USING_OPENGL
	bindings.add({index, Binding::Type::StorageBuffer, buffer});
#endif
#ifdef USING_VULKAN
	dset->set_storage_buffer(index, buffer);
	dset->update();
#endif
}

void BindingData::apply(Shader* shader, const RenderParams& params) {
#ifdef USING_OPENGL
	for (auto& b: bindings) {
		if (b.type == Binding::Type::Texture)
			nix::bind_texture(b.index, static_cast<Texture*>(b.p));
		else if (b.type == Binding::Type::Image)
			nix::bind_image(b.index, static_cast<nix::ImageTexture*>(b.p), 0, 0, true);
		else if (b.type == Binding::Type::UniformBuffer)
			nix::bind_uniform_buffer(b.index, static_cast<nix::UniformBuffer*>(b.p));
		else if (b.type == Binding::Type::StorageBuffer)
			nix::bind_storage_buffer(b.index, static_cast<nix::ShaderStorageBuffer*>(b.p));
	}
	apply_shader_data(shader, shader_data);
#else
    auto cb = params.command_buffer;
	cb->bind_descriptor_set(0, dset.get());
	apply_shader_data(cb, shader_data);
#endif
}

}

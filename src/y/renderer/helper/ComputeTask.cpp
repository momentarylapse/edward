#include "ComputeTask.h"

#include <renderer/base.h>

#include "../../graphics-impl.h"


ComputeTask::ComputeTask(const shared<Shader>& _shader) {
    shader = _shader;
#ifdef USING_VULKAN
    pool = new vulkan::DescriptorPool("sampler:8,buffer:8,storage-buffer:8,image:8", 1);
    dset = pool->create_set_from_layout(shader->descr_layouts[0]);
    pipeline = new vulkan::ComputePipeline(shader.get());
#endif
}

#ifdef USING_OPENGL
void ComputeTask::dispatch(int nx, int ny, int nz) {
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
    shader->dispatch(nx, ny, nz);
    nix::image_barrier();
}
#endif
#ifdef USING_VULKAN
void ComputeTask::dispatch(CommandBuffer* cb, int nx, int ny, int nz) {
    cb->set_bind_point(vulkan::PipelineBindPoint::COMPUTE);
    cb->bind_pipeline(pipeline.get());
    cb->bind_descriptor_set(0, dset.get());
    cb->dispatch(nx, ny, nz);
    cb->set_bind_point(vulkan::PipelineBindPoint::GRAPHICS);
    gpu_flush();
}
#endif

void ComputeTask::bind_texture(int index, Texture *texture) {
#ifdef USING_OPENGL
    bindings.add({index, Binding::Type::Texture, texture});
#endif
#ifdef USING_VULKAN
    dset->set_texture(index, texture);
    dset->update();
#endif
}

void ComputeTask::bind_image(int index, ImageTexture *texture) {
#ifdef USING_OPENGL
    bindings.add({index, Binding::Type::Image, texture});
#endif
#ifdef USING_VULKAN
    dset->set_storage_image(index, texture);
    dset->update();
#endif
}

void ComputeTask::bind_uniform_buffer(int index, Buffer *buffer) {
#ifdef USING_OPENGL
    bindings.add({index, Binding::Type::UniformBuffer, buffer});
#endif
#ifdef USING_VULKAN
    dset->set_buffer(index, buffer);
    dset->update();
#endif
}

void ComputeTask::bind_storage_buffer(int index, Buffer *buffer) {
#ifdef USING_OPENGL
    bindings.add({index, Binding::Type::StorageBuffer, buffer});
#endif
#ifdef USING_VULKAN
    dset->set_storage_buffer(index, buffer);
    dset->update();
#endif
}



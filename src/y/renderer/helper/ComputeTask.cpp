#include "ComputeTask.h"
#include <renderer/base.h>
#include "../../graphics-impl.h"
#include "../../helper/PerformanceMonitor.h"



ComputeTask::ComputeTask(const string& name, const shared<Shader>& _shader, int _nx, int _ny, int _nz) :
    RenderTask(name),
    bindings(_shader.get())
{
    shader = _shader;
    nx = _nx;
    ny = _ny;
    nz = _nz;
#ifdef USING_VULKAN
    if (name != "") {
        pipeline = new vulkan::ComputePipeline(shader.get());
    }
#endif
}

void ComputeTask::render(const RenderParams &params) {
	PerformanceMonitor::begin(channel);
    gpu_timestamp_begin(params, channel);
#ifdef USING_OPENGL
    bindings.apply(shader.get(), params);
    shader->dispatch(nx, ny, nz);
    nix::image_barrier();
#endif
#ifdef USING_VULKAN
    auto cb = params.command_buffer;
    cb->set_bind_point(vulkan::PipelineBindPoint::COMPUTE);
    cb->bind_pipeline(pipeline.get());
    bindings.apply(shader.get(), params);
    cb->dispatch(nx, ny, nz);
    cb->set_bind_point(vulkan::PipelineBindPoint::GRAPHICS);
    // TODO barriers
#endif
    gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
}



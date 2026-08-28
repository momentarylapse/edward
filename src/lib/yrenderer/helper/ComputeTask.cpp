#include "ComputeTask.h"
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/profiler/Profiler.h>


namespace yrenderer {

ComputeTask::ComputeTask(Context* ctx, const string& name, const shared<ygfx::Shader>& _shader, int _nx, int _ny, int _nz) :
	RenderTask(ctx, name),
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
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);
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
	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

ComputeScheduler::ComputeScheduler(Context *ctx) : RenderTask(ctx, "ComputeScheduler") {
#ifdef USING_VULKAN
	command_buffer = new vulkan::CommandBuffer(ctx->device->command_pool);
	fence = new vulkan::Fence(ctx->device);
#endif
}

ComputeScheduler::~ComputeScheduler() = default;

RenderParams ComputeScheduler::start() {
	RenderParams params = RenderParams::WHATEVER;
#ifdef USING_VULKAN
	params.command_buffer = command_buffer.get();
	command_buffer->begin();
#endif
	return params;
}

void ComputeScheduler::submit(bool block) {
#ifdef USING_VULKAN
	command_buffer->end();
	ctx->device->compute_queue.submit(command_buffer.get(), {}, {}, fence.get());
#endif
	if (block)
		wait();
}

void ComputeScheduler::wait() {
#ifdef USING_VULKAN
	fence->wait();
	//ctx->device->wait_idle();
#endif
}

}



//
// Created by michi on 12/29/24.
//

#include "LightMeter.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>
#include <lib/profiler/Profiler.h>
#include <lib/ygraphics/ShaderManager.h>
#include <cmath>

namespace yrenderer {

constexpr int NBINS = 256;
constexpr int NSAMPLES = 2560;

using namespace ygfx;

LightMeter::LightMeter(Context* ctx, Texture* tex)
	: ComputeTask(ctx, "expo", REQUIRED(ctx->shader_manager->load_shader("compute/brightness.shader")), NSAMPLES, 1, 1)
{
	ch_prepare = profiler::create_channel("expo.p", channel);
	params = new UniformBuffer(8);
	buf = new ShaderStorageBuffer(NBINS*4);
	texture = tex;
	bind_texture(0, tex);
	bind_storage_buffer(1, buf);
	bind_uniform_buffer(2, params);
}

void LightMeter::set_texture(ygfx::Texture* tex) {
	if (tex == texture)
		return;
	texture = tex;
	bind_texture(0, tex);
}

Array<int> LightMeter::read_histogram() {
	profiler::begin(ch_prepare);

	// TODO barriers...
	ctx->gpu_flush();
	Array<int> histogram;
	if (cur_histogram.num == NBINS) {
#ifdef USING_VULKAN
		void* p = buf->map();
		memcpy(&cur_histogram[0], p, NBINS*sizeof(int));
		buf->unmap();
#else
		buf->read(&cur_histogram[0], NBINS*sizeof(int));
#endif
		//msg_write(str(cur_histogram));
		histogram = cur_histogram;
	}

	cur_histogram.resize(NBINS);
	memset(&cur_histogram[0], 0, NBINS * sizeof(int));
	buf->update_array(cur_histogram);
	setup();
	profiler::end(ch_prepare);
	return histogram;
}

void LightMeter::setup() {
	int pp[2] = {texture->width, texture->height};
#ifdef USING_VULKAN
	params->update(&pp);
#else
	params->update(&pp, sizeof(pp));
#endif
}

}

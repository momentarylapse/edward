//
// Created by michi on 12/29/24.
//

#include "LightMeter.h"
#include <world/Camera.h>
#include "../../graphics-impl.h"
#include "../base.h"
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"


constexpr int NBINS = 256;
constexpr int NSAMPLES = 2560;

LightMeter::LightMeter(ResourceManager* resource_manager, Texture* tex)
	: ComputeTask("expo", resource_manager->load_shader("compute/brightness.shader"), NSAMPLES, 1, 1)
{
	ch_prepare = PerformanceMonitor::create_channel("expo.p", channel);
	params = new UniformBuffer(8);
	buf = new ShaderStorageBuffer(NBINS*4);
	texture = tex;
	bind_texture(0, tex);
	bind_storage_buffer(1, buf);
	bind_uniform_buffer(2, params);
	brightness = 1;
}

void LightMeter::read() {
	PerformanceMonitor::begin(ch_prepare);

	// TODO barriers...
	gpu_flush();
	if (histogram.num == NBINS) {
#ifdef USING_VULKAN
		void* p = buf->map();
		memcpy(&histogram[0], p, NBINS*sizeof(int));
		buf->unmap();
#else
		buf->read(&histogram[0], NBINS*sizeof(int));
#endif
		//msg_write(str(histogram));

		int thresh = (NSAMPLES * 16 * 16) / 200 * 199;
		int n = 0;
		int ii = 0;
		for (int i=0; i<NBINS; i++) {
			n += histogram[i];
			if (n > thresh) {
				ii = i;
				break;
			}
		}
		brightness = pow(2.0f, ((float)ii / (float)NBINS) * 20.0f - 10.0f);
	}

	histogram.resize(NBINS);
	memset(&histogram[0], 0, NBINS * sizeof(int));
	buf->update_array(histogram);
	PerformanceMonitor::end(ch_prepare);
}

void LightMeter::setup() {
	int pp[2] = {texture->width, texture->height};
#ifdef USING_VULKAN
	params->update(&pp);
#else
	params->update(&pp, sizeof(pp));
#endif
}

void LightMeter::adjust_camera(Camera *cam) {
	float exposure = clamp((float)pow(1.0f / brightness, 0.8f), cam->auto_exposure_min, cam->auto_exposure_max);
	if (exposure > cam->exposure)
		cam->exposure *= 1.05f;
	if (exposure < cam->exposure)
		cam->exposure /= 1.05f;
}

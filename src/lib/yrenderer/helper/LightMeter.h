//
// Created by michi on 12/29/24.
//

#ifndef LIGHTMETER_H
#define LIGHTMETER_H

#include "../Renderer.h"
#include "ComputeTask.h"

namespace yrenderer {

class LightMeter : public yrenderer::ComputeTask {
public:
	LightMeter(Context* ctx, ygfx::Texture* tex);
	ygfx::UniformBuffer* params;
	ygfx::ShaderStorageBuffer* buf;
	Array<int> histogram;
	float brightness;
	ygfx::Texture* texture;
	void read();
	void setup();
	void adjust_camera(float* exposure, float exposure_min, float exposure_max);

	int ch_prepare;
};

}


#endif //LIGHTMETER_H

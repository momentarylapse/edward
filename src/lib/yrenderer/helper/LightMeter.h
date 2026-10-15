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
	void set_texture(ygfx::Texture* tex);
	ygfx::UniformBuffer* params;
	ygfx::ShaderStorageBuffer* buf;
	Array<int> cur_histogram;
	ygfx::Texture* texture;
	Array<int> read_histogram();
	void setup();

	int ch_prepare;
};

}


#endif //LIGHTMETER_H

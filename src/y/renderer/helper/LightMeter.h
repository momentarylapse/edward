//
// Created by michi on 12/29/24.
//

#ifndef LIGHTMETER_H
#define LIGHTMETER_H

#include "../Renderer.h"
#include "../helper/ComputeTask.h"

class ComputeTask;
class Camera;

class LightMeter : public ComputeTask {
public:
	LightMeter(ResourceManager* resource_manager, Texture* tex);
	UniformBuffer* params;
	ShaderStorageBuffer* buf;
	Array<int> histogram;
	float brightness;
	Texture* texture;
	void read();
	void setup();
	void adjust_camera(Camera* cam);
};



#endif //LIGHTMETER_H

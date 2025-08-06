//
// Created by michi on 04.11.23.
//

#ifndef Y_SCENEVIEW_H
#define Y_SCENEVIEW_H

#include "CameraParams.h"
#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/math/vec3.h>


struct RayTracingData;

namespace yrenderer {

struct Light;
struct UBOLight;

static constexpr int MAX_LIGHTS = 1024 - 24; // :P

// "scene" rendered by 1 camera
//   mostly lighting situation
//   includes shadow/cube maps from multiple perspectives
//   (might be shared with nearby cameras)
struct SceneView {
	CameraParams main_camera_params;
	Array<ygfx::DepthBuffer*> shadow_maps;
	shared<ygfx::CubeMap> cube_map;
	Array<Light*> lights;
	Array<int> shadow_indices;
	owned<ygfx::UniformBuffer> surfel_buffer;
	int num_surfels = 0;
	ivec3 probe_cells;
	vec3 probe_min, probe_max;
	RayTracingData* ray_tracing_data = nullptr;

	void choose_lights(const Array<Light*>& all_lights);
	void choose_shadows();
};

}


#endif //Y_SCENEVIEW_H

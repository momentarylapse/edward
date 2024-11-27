//
// Created by michi on 04.11.23.
//

#ifndef Y_SCENEVIEW_H
#define Y_SCENEVIEW_H

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/mat4.h>
#include <graphics-fwd.h>

class Camera;
struct UBOLight;
struct XTerrainVBUpdater;

class TerrainUpdateThread;

struct SceneView {
	Camera *cam;
	Array<UBOLight> lights;
	shared<FrameBuffer> fb_shadow1;
	shared<FrameBuffer> fb_shadow2;
	shared<CubeMap> cube_map;
	int shadow_index = -1;
	mat4 shadow_proj;

	void prepare_lights(float shadow_box_size, UniformBuffer* ubo_light);


	void check_terrains(const vec3& cam_pos);

	//TerrainUpdateThread* terrain_update_thread = nullptr;
	Array<XTerrainVBUpdater*> updater;
};


#endif //Y_SCENEVIEW_H

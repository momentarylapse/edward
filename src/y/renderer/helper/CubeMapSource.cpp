#include "CubeMapSource.h"
#include <Config.h>
#include <graphics-impl.h>
#include <lib/os/msg.h>
#include <renderer/base.h>
#include <renderer/path/RenderPath.h>
#include <renderer/scene/SceneView.h>
#include <renderer/scene/SceneRenderer.h>
#include <renderer/target/TextureRenderer.h>
#include <y/Entity.h>

#include "world/Camera.h"
#include "world/World.h"

const kaba::Class* CubeMapSource::_class = nullptr;;

CubeMapSource::CubeMapSource() {
	min_depth = 1.0f;
	max_depth = 100000.0f;
	resolution = config.get_int("cubemap.resolution", 256);
	update_rate = config.get_int("cubemap.update_rate", 9);
	counter = 0;
}

CubeMapSource::~CubeMapSource() = default;


CubeMapRenderer::CubeMapRenderer(SceneView& scene_view, shared_array<MeshEmitter> emitters) : RenderTask("cube") {
	for (int i=0; i<6; i++) {
		scene_renderers[i] = new SceneRenderer(RenderPathType::Forward, scene_view);
		for (auto e: weak(emitters))
			scene_renderers[i]->add_emitter(e);
	}
}

void CubeMapRenderer::set_source(CubeMapSource* s) {
	source = s;
}


void CubeMapRenderer::render(const RenderParams& params) {
	gpu_timestamp_begin(params, channel);
	if (!source->depth_buffer)
		source->depth_buffer = new DepthBuffer(source->resolution, source->resolution, "ds:u24i8");
	if (!source->cube_map)
		source->cube_map = new CubeMap(source->resolution, "rgba:i8");
	if (!texture_renderers[0])
		for (int i=0; i<6; i++) {
			texture_renderers[i] = new TextureRenderer("tex", {source->cube_map.get(), source->depth_buffer.get()});
			texture_renderers[i]->set_layer(i);
			texture_renderers[i]->add_child(scene_renderers[i].get());
		}

	//mat4 proj = mat4::perspective(pi/2, 1, source->min_depth, source->max_depth, false) * mat4::scale(1,-1,1);
	mat4 proj = mat4::perspective(pi/2, 1, cam_main->min_depth, cam_main->max_depth, false) * mat4::scale(1,-1,1);

	for (int i=0; i<6; i++) {
		quaternion ang;
		if (i == 0)
			ang = quaternion::rotation(vec3(0,pi/2,0));
		if (i == 1)
			ang = quaternion::rotation(vec3(0,-pi/2,0));
		if (i == 2)
			ang = quaternion::rotation(vec3(-pi/2,pi,pi));
		if (i == 3)
			ang = quaternion::rotation(vec3(pi/2,pi,pi));
		if (i == 4)
			ang = quaternion::rotation(vec3(0,0,0));
		if (i == 5)
			ang = quaternion::rotation(vec3(0,pi,0));

		scene_renderers[i]->set_view(texture_renderers[i]->make_params(params), source->owner->pos, ang, proj);
		//prepare_lights(&cam);
		//render_into_texture(source->frame_buffer[i].get(), &cam, rvd[i]);

		texture_renderers[i]->render(params);
	}
	gpu_timestamp_end(params, channel);
}




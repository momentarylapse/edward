//
// Created by michi on 08.08.25.
//

#include "CubeMapRenderer.h"
#include "../../helper/CubeMapSource.h"
#include "../CameraParams.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include <lib/yrenderer/target/TextureRenderer.h>

namespace yrenderer {

CubeMapRenderer::CubeMapRenderer(Context* ctx, SceneView& scene_view) : RenderTask(ctx, "cube") {
	for (int i=0; i<6; i++)
		scene_renderers[i] = new SceneRenderer(ctx, RenderPathType::Forward, scene_view);
}

void CubeMapRenderer::set_source(CubeMapSource* s) {
	source = s;
}

void CubeMapRenderer::add_emitter(shared<MeshEmitter> emitter) {
	for (int i=0; i<6; i++)
		scene_renderers[i]->add_emitter(emitter);
}



void CubeMapRenderer::render(const RenderParams& params) {
	ctx->gpu_timestamp_begin(params, channel);
	if (!source->depth_buffer)
		source->depth_buffer = new ygfx::DepthBuffer(source->resolution, source->resolution, "ds:u24i8");
	if (!source->cube_map)
		source->cube_map = new ygfx::CubeMap(source->resolution, "rgba:i8");
	if (!texture_renderers[0])
		for (int i=0; i<6; i++) {
			texture_renderers[i] = new TextureRenderer(ctx, "tex", {source->cube_map.get(), source->depth_buffer.get()});
			texture_renderers[i]->set_layer(i);
			texture_renderers[i]->add_child(scene_renderers[i].get());
		}

	//mat4 proj = mat4::perspective(pi/2, 1, source->min_depth, source->max_depth, false) * mat4::scale(1,-1,1);
	mat4 proj = mat4::perspective(pi/2, 1, source->min_depth, source->max_depth, false) * mat4::scale(1,-1,1);

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

		scene_renderers[i]->set_view(texture_renderers[i]->make_params(params), {source->pos, ang, pi/2, source->min_depth, source->max_depth}, &proj);

		texture_renderers[i]->render(params);
	}
	ctx->gpu_timestamp_end(params, channel);
}

}


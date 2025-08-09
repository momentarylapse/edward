#pragma once

#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/pointer.h>
#include <lib/yrenderer/scene/RenderViewData.h>

namespace yrenderer {

extern int cubemap_default_resolution;
extern int cubemap_default_rate;


class CubeMapSource {
public:
	CubeMapSource();
	~CubeMapSource();

	vec3 pos;
	float min_depth, max_depth;
	shared<ygfx::CubeMap> cube_map;
	shared<ygfx::DepthBuffer> depth_buffer;
	shared<ygfx::FrameBuffer> frame_buffer[6];
#ifdef USING_VULKAN
	owned<ygfx::RenderPass> render_pass;
#endif
	int resolution;
	int update_rate;
	int counter;
};

}

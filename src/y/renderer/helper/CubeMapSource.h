#ifndef CUBEMAPSOURCE_H
#define CUBEMAPSOURCE_H

#include <y/Component.h>
#include <graphics-fwd.h>
#include <lib/base/pointer.h>
#include "../world/geometry/RenderViewData.h"

class CubeMapSource : public Component {
public:
	CubeMapSource();
	~CubeMapSource() override;

	float min_depth, max_depth;
	shared<CubeMap> cube_map;
	shared<DepthBuffer> depth_buffer;
	shared<FrameBuffer> frame_buffer[6];
#ifdef USING_VULKAN
	owned<vulkan::RenderPass> render_pass;
#endif
	RenderViewData rvd[6];
	int resolution;
	int update_rate;
	int counter;

	static const kaba::Class *_class;
};

#endif //CUBEMAPSOURCE_H

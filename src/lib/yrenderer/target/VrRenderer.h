//
// Created by michi on 8/17/26.
//

#pragma once
#if __has_include(<lib/vr/vr.h>)
#if HAS_LIB_OPENXR
#include <lib/ygraphics/graphics-fwd.h>
#ifdef USING_VULKAN
#define HAS_VR_RENDERER 1

#include "WindowRenderer.h"

namespace yrenderer {


class VrRenderer : public TargetRenderer {
public:
	VrRenderer(Context* ctx);

	bool start_frame();
	void end_frame();
	void start_view(int index);
	void end_view();

	void prepare(const RenderParams& params) override;
	void draw(const RenderParams& params) override;

	RenderParams create_params();

	owned<Fence> in_flight_fence;
	owned<ygfx::CommandBuffer> command_buffer;
	owned<vulkan::RenderPass> render_pass;
	Device *device;
	bool gamma_correction;
	int current_view_index = 0;
};

}

#endif
#endif
#endif

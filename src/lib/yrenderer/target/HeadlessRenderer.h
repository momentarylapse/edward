//
// Created by michi on 1/12/25.
//

#pragma once

#include "TextureRenderer.h"

namespace yrenderer {

// TODO "task executor"...
class HeadlessRenderer : public RenderTask {
public:
#ifdef USING_VULKAN
	vulkan::Device* device;
	ygfx::CommandBuffer* command_buffer;
	vulkan::Fence* fence;
#endif

	owned<TextureRenderer> texture_renderer;

	HeadlessRenderer(Context* ctx, const shared_array<ygfx::Texture>& tex);
	~HeadlessRenderer() override;

	void render(const RenderParams& params) override;

	RenderParams create_params(const rect& area) const;
};

}

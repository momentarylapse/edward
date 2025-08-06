#if __has_include(<lib/hui/hui.h>)
#pragma once

#include "../Renderer.h"

namespace yrenderer {

#ifdef USING_OPENGL

class HuiWindowRenderer : public Renderer {
	Context* ctx;

public:
	explicit HuiWindowRenderer(Context* _ctx);
	void render_frame();
};
#endif

#ifdef USING_VULKAN
#include <lib/image/image.h>

class TextureRenderer;

class HuiWindowRenderer : public Renderer {
public:
	static constexpr int MAX_WIDTH = 1024*3;
	static constexpr int MAX_HEIGHT = 2048;
	shared<Texture> texture;
	explicit HuiWindowRenderer(vulkan::Instance* instance);
	void prepare(const RenderParams& p) override;
	void render_frame(Painter* p);
	RenderParams create_params(int w, int h);

	vulkan::Device* device;
	TextureRenderer* texture_renderer = nullptr;
	CommandBuffer* command_buffer = nullptr;
	vulkan::Fence* fence = nullptr;
	Image image;
};
#endif

}

#endif

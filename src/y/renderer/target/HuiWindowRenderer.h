#if __has_include(<lib/hui/hui.h>)
#pragma once

#include "../Renderer.h"
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

class TextureRendererVulkan;

class HuiWindowRenderer : public Renderer {
public:
	static constexpr int MAX_WIDTH = 1024*3;
	static constexpr int MAX_HEIGHT = 2048;
	shared<Texture> texture;
	explicit HuiWindowRenderer(vulkan::Instance* instance);
	void prepare(const RenderParams& p);
	void render_frame(Painter* p);
	RenderParams create_params(int w, int h);

	vulkan::Device* device;
	TextureRendererVulkan* texture_renderer = nullptr;
	CommandBuffer* cb = nullptr;
	Image image;
};
#endif
#endif

/*
 * GuiRenderer.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>

namespace ygfx {
	struct DrawingHelperData;
}

class GuiRenderer : public yrenderer::Renderer {
public:
	explicit GuiRenderer(yrenderer::Context* ctx);

	void prepare(const yrenderer::RenderParams &params) override;
	void draw(const yrenderer::RenderParams& params) override;

	shared<ygfx::Shader> shader;
#ifdef USING_VULKAN
	ygfx::GraphicsPipeline* pipeline = nullptr;
	Array<ygfx::DescriptorSet*> dset;
	Array<ygfx::UniformBuffer*> ubo;
#endif
	owned<ygfx::VertexBuffer> vb;

	owned<ygfx::DrawingHelperData> aux;

#ifdef USING_VULKAN
	void prepare_gui(ygfx::FrameBuffer *source, const yrenderer::RenderParams& params);
	void draw_gui(ygfx::CommandBuffer *cb, const yrenderer::RenderParams& params);
#endif
};


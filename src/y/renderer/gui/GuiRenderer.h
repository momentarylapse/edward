/*
 * GuiRenderer.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>

class GuiRenderer : public yrenderer::Renderer {
public:
	explicit GuiRenderer(yrenderer::Context* ctx);

	void draw(const yrenderer::RenderParams& params) override;

	shared<ygfx::Shader> shader;
#ifdef USING_VULKAN
	ygfx::GraphicsPipeline* pipeline = nullptr;
	Array<ygfx::DescriptorSet*> dset;
	Array<ygfx::UniformBuffer*> ubo;
#endif
	owned<ygfx::VertexBuffer> vb;

#ifdef USING_VULKAN
	void prepare_gui(ygfx::FrameBuffer *source, const yrenderer::RenderParams& params);
	void draw_gui(ygfx::CommandBuffer *cb, ygfx::RenderPass *render_pass);
#endif
};


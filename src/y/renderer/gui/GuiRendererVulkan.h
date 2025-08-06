/*
 * GuiRendererVulkan.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include <lib/yrenderer/Renderer.h>
#ifdef USING_VULKAN

class GuiRendererVulkan : public yrenderer::Renderer {
public:
	explicit GuiRendererVulkan(yrenderer::Context* ctx);

	void draw(const yrenderer::RenderParams& params) override;

	shared<ygfx::Shader> shader;
	ygfx::GraphicsPipeline* pipeline = nullptr;
	Array<ygfx::DescriptorSet*> dset;
	Array<ygfx::UniformBuffer*> ubo;
	owned<ygfx::VertexBuffer> vb;
	void prepare_gui(ygfx::FrameBuffer *source, const yrenderer::RenderParams& params);
	void draw_gui(ygfx::CommandBuffer *cb, ygfx::RenderPass *render_pass);
};

#endif

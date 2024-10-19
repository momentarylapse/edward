/*
 * GuiRendererVulkan.h
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#pragma once

#include "../Renderer.h"
#ifdef USING_VULKAN

class GuiRendererVulkan : public Renderer {
public:
	GuiRendererVulkan();

	void draw(const RenderParams& params) override;

	shared<Shader> shader;
	GraphicsPipeline* pipeline = nullptr;
	Array<DescriptorSet*> dset;
	Array<UniformBuffer*> ubo;
	owned<VertexBuffer> vb;
	void prepare_gui(FrameBuffer *source, const RenderParams& params);
	void draw_gui(CommandBuffer *cb, RenderPass *render_pass);
};

#endif

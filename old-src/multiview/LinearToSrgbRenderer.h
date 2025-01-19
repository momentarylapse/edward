//
// Created by Michael Ankele on 2024-11-22.
//

#ifndef LINEARTOSRGBRENDERER_H
#define LINEARTOSRGBRENDERER_H

#include <renderer/Renderer.h>

class Session;


class LinearToSrgbRenderer : public ::Renderer {
public:
	Session* session;
	explicit LinearToSrgbRenderer(Session* _session);
	void ensure_fb_size(const rect& r);
	void prepare(const RenderParams& p) override;
	void draw(const RenderParams& p) override;

	shared<FrameBuffer> frame_buffer;
	shared<Shader> shader_out;
#if HAS_LIB_VULKAN
	GraphicsPipeline* pipeline_out = nullptr;
	DescriptorSet *dset_out = nullptr;
	RenderPass* render_pass = nullptr;
#endif
	VertexBuffer *vb_2d = nullptr;
	rect vb_2d_current_source = rect::EMPTY;
};

#endif //LINEARTOSRGBRENDERER_H

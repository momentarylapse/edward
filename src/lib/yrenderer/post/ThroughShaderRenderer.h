//
// Created by michi on 12/8/24.
//

#ifndef THROUGHSHADERRENDERER_H
#define THROUGHSHADERRENDERER_H

#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/helper/Bindable.h>
#include <lib/any/any.h>
#include <lib/math/rect.h>

namespace yrenderer {

class ThroughShaderRenderer : public Renderer {
public:
	ThroughShaderRenderer(Context* ctx, const string& name, shared<ygfx::Shader> shader);
	void draw(const RenderParams &params) override;

	void set_source(const rect& area);

	IMPLEMENT_BINDABLE_INTERFACE

	shared<ygfx::Shader> shader;
	owned<ygfx::VertexBuffer> vb_2d;
	rect current_area;

#ifdef USING_VULKAN
	ygfx::GraphicsPipeline* pipeline = nullptr;
#endif
};

}

#endif //THROUGHSHADERRENDERER_H

//
// Created by michi on 12/8/24.
//

#ifndef THROUGHSHADERRENDERER_H
#define THROUGHSHADERRENDERER_H

#include "../Renderer.h"
#include "../helper/Bindable.h"
#include <lib/any/any.h>
#include <lib/math/rect.h>

class ThroughShaderRenderer : public Renderer {
public:
	ThroughShaderRenderer(const string& name, shared<Shader> _shader);
	void draw(const RenderParams &params) override;

	void set_source(const rect& area);

	IMPLEMENT_BINDABLE_INTERFACE

	shared<Shader> shader;
	owned<VertexBuffer> vb_2d;
	rect current_area;

#ifdef USING_VULKAN
	GraphicsPipeline* pipeline = nullptr;
#endif
};

#endif //THROUGHSHADERRENDERER_H

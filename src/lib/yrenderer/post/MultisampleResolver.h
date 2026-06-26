//
// Created by michi on 12/8/24.
//

#ifndef MULTISAMPLERESOLVER_H
#define MULTISAMPLERESOLVER_H

#include <lib/yrenderer/Renderer.h>

namespace yrenderer {

class ThroughShaderRenderer;
class TextureRenderer;

class MultisampleResolver : public Renderer {
public:
	MultisampleResolver(Context* ctx, int width, int height, int samples);

	void prepare(const RenderParams& params) override;

	shared<ygfx::TextureMultiSample> texture;
	shared<ygfx::TextureMultiSample> depth_buffer;

	owned<ThroughShaderRenderer> out_renderer;
	owned<TextureRenderer> texture_renderer;

	shared<ygfx::Shader> shader_resolve_multisample;
};

}

#endif //MULTISAMPLERESOLVER_H

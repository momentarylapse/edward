//
// Created by michi on 12/8/24.
//

#ifndef MULTISAMPLERESOLVER_H
#define MULTISAMPLERESOLVER_H

#include <lib/yrenderer/Renderer.h>

namespace yrenderer {

class ThroughShaderRenderer;
class TextureRenderer;

class MultisampleResolver : public RenderTask {
public:
	MultisampleResolver(Context* ctx, ygfx::Texture* tex_ms, ygfx::Texture* depth_ms, ygfx::Texture* tex_out, ygfx::Texture* depth_out);

	void render(const RenderParams& params) override;

	owned<ThroughShaderRenderer> tsr;
	owned<TextureRenderer> into_texture;
	shared<ygfx::Shader> shader_resolve_multisample;
};

}

#endif //MULTISAMPLERESOLVER_H

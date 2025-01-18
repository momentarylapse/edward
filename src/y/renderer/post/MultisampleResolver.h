//
// Created by michi on 12/8/24.
//

#ifndef MULTISAMPLERESOLVER_H
#define MULTISAMPLERESOLVER_H

#include "renderer/Renderer.h"

class ThroughShaderRenderer;
class TextureRenderer;

class MultisampleResolver : public RenderTask {
public:
	MultisampleResolver(Texture* tex_ms, Texture* depth_ms, Texture* tex_out, Texture* depth_out);

	void render(const RenderParams& params) override;

	owned<ThroughShaderRenderer> tsr;
	owned<TextureRenderer> into_texture;
	shared<Shader> shader_resolve_multisample;
};

#endif //MULTISAMPLERESOLVER_H

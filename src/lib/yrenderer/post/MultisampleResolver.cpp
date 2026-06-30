#include "MultisampleResolver.h"
#include "ThroughShaderRenderer.h"
#include "../target/TextureRenderer.h"
#include <lib/ygraphics/ShaderManager.h>
#include <lib/ygraphics/graphics-impl.h>

// https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing


namespace nix {
	void resolve_multisampling(ygfx::FrameBuffer *target, ygfx::FrameBuffer *source);
}

namespace yrenderer {

MultisampleResolver::MultisampleResolver(Context* ctx, int width, int height, int samples) : Renderer(ctx, "ms") {
	texture = new ygfx::TextureMultiSample(width, height, samples, "rgba:f16");
	depth_buffer = new ygfx::TextureMultiSample(width, height, samples, "d:f32");
	//depth_buffer = new nix::RenderBuffer(width, height, samples, "ds:u24i88");
	texture_renderer = new TextureRenderer(ctx, "tex", {texture.get(), depth_buffer.get()}, {format("samples=%d", samples)});

	shader_resolve_multisample = shader_manager->load_shader("post/resolve-multisample.shader");
	out_renderer = new ThroughShaderRenderer(ctx, "ms", shader_resolve_multisample);
	out_renderer->bind_textures(0, {texture.get(), depth_buffer.get()});
	add_child(out_renderer.get());

	custodian = texture_renderer.get();
}

void MultisampleResolver::prepare(const RenderParams& params) {
	// resolve
	if (true) {
		out_renderer->bindings.shader_data.dict_set("width:0", Any((float)texture_renderer->frame_buffer->width));
		out_renderer->bindings.shader_data.dict_set("height:4", Any((float)texture_renderer->frame_buffer->height));
		out_renderer->set_source(dynamicly_scaled_source());
		texture_renderer->set_area(dynamicly_scaled_area(texture_renderer->frame_buffer.get()));
		texture_renderer->render(params);
	} else {
		// not sure, why this does not work... :(
		//			nix::resolve_multisampling(fb_main.get(), fb_main_ms.get());
	}
}

}


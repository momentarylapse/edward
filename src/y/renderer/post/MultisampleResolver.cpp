#include "MultisampleResolver.h"
#include "ThroughShaderRenderer.h"
#include "../target/TextureRenderer.h"
#include "../../helper/ResourceManager.h"
#include "../../graphics-impl.h"

// https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing


namespace nix {
	void resolve_multisampling(FrameBuffer *target, FrameBuffer *source);
}

MultisampleResolver::MultisampleResolver(Texture* tex_ms, Texture* depth_ms, Texture* tex_out, Texture* depth_out) : RenderTask("ms") {
	shader_resolve_multisample = resource_manager->load_shader("forward/resolve-multisample.shader");
	tsr = new ThroughShaderRenderer("ms", shader_resolve_multisample);
	tsr->bind_textures(0, {tex_ms, depth_ms});

	into_texture = new TextureRenderer("tex", {tex_out, depth_out});
	into_texture->add_child(tsr.get());
}

void MultisampleResolver::render(const RenderParams& params) {
	// resolve
	if (true) {
		tsr->bindings.shader_data.dict_set("width:0", into_texture->frame_buffer->width);
		tsr->bindings.shader_data.dict_set("height:4", into_texture->frame_buffer->height);
		tsr->set_source(dynamicly_scaled_source());
		into_texture->set_area(dynamicly_scaled_area(into_texture->frame_buffer.get()));
		into_texture->render(params);
	} else {
		// not sure, why this does not work... :(
		//			nix::resolve_multisampling(fb_main.get(), fb_main_ms.get());
	}
}


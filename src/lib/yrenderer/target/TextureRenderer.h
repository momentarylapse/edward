/*
 * TextureRenderer.h
 *
 *  Created on: Nov 10, 2023
 *      Author: michi
 */

#pragma once


#include <lib/yrenderer/Renderer.h>
#include <lib/base/optional.h>
#include <lib/image/color.h>

namespace yrenderer {

class TextureRenderer : public RenderTask {
public:
	explicit TextureRenderer(Context* ctx, const string& name, const shared_array<ygfx::Texture>& textures, const Array<string>& options = {});
	~TextureRenderer() override;

	RenderParams make_params(const RenderParams& params) const;

	void render(const RenderParams& params) override;

	void set_area(const rect& area);
	bool override_area = false;
	rect user_area;

	void set_layer(int layer);

	shared<ygfx::FrameBuffer> frame_buffer;
#ifdef USING_VULKAN
	owned<ygfx::RenderPass> render_pass;
#endif
	shared_array<ygfx::Texture> textures;

	bool clear_z = true;
	Array<color> clear_colors;
};

}


#if __has_include(<lib/hui/hui.h>)
#include "HuiWindowRenderer.h"
#ifdef USING_VULKAN
#include "TextureRendererVulkan.h"
#include "../../graphics-impl.h"
#include <lib/hui/hui.h>

HuiWindowRenderer::HuiWindowRenderer(vulkan::Instance* instance) : Renderer("hui") {
	device = vulkan::Device::create_simple(instance, nullptr, {"graphics", "anisotropy"});

	image.create(MAX_WIDTH, MAX_HEIGHT, Red);
	texture = new Texture(MAX_WIDTH, MAX_HEIGHT, "bgra:i8");
	texture_renderer = new TextureRendererVulkan(device, texture);
}

void HuiWindowRenderer::prepare(const RenderParams& p) {
	texture_renderer->children = children;
	texture_renderer->render_frame(p.area, p.desired_aspect_ratio);
}

void HuiWindowRenderer::render_frame(Painter* p) {
	int scale = hui::get_event()->row_target;
	int w = p->width * scale;
	int h = p->height * scale;

	const auto params = create_params(w, h);
	prepare(params);

	texture->read(&image.data[0]);
	//image.mode = Image::Mode::BGRA;
	//image.set_mode(Image::Mode::RGBA);

	float t[4] = {1 / (float)scale, 0, 0, 1 / (float)scale};
	p->set_transform(t, {0,0});
	p->draw_image({0, 0}, &image);
}

RenderParams HuiWindowRenderer::create_params(int w, int h) {
	return texture_renderer->create_params({0, (float)w, 0, (float)h});
}

#endif
#endif

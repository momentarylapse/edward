//
// Created by Michael Ankele on 2024-11-22.
//

#include "LinearToSrgbRenderer.h"
#include <y/graphics-impl.h>
#include <y/helper/ResourceManager.h>
#include "../Session.h"


LinearToSrgbRenderer::LinearToSrgbRenderer(Session* _session) : ::Renderer("lin2srgb") {
	session = _session;
	shader_out = session->resource_manager->load_shader("multiview-out.shader");
	vb_2d = new VertexBuffer("3f,3f,2f");
#if HAS_LIB_VULKAN
	auto pool = new vulkan::DescriptorPool("sampler:1", 1);
	dset_out = pool->create_set("sampler");
#endif
}

void LinearToSrgbRenderer::ensure_fb_size(const rect& r) {
	// we should not re-create when shrinking... but then we would need to
	// think harder about some transformations etc...
	if (frame_buffer)
		if (frame_buffer->width == r.width() and frame_buffer->height == r.height())
			return;
#if HAS_LIB_GL
	auto depth_buffer = new DepthBuffer(r.width(), r.height(), "d24s8");
	auto tex = new Texture(r.width(), r.height(), "rgba:f32");
	frame_buffer = new FrameBuffer({tex, depth_buffer});
#endif
#if HAS_LIB_VULKAN
	auto depth_buffer = new DepthBuffer(r.width(), r.height(), "ds:f32i8", false);
	auto tex = new Texture(r.width(), r.height(), "rgba:f32");
	render_pass = new vulkan::RenderPass({tex, depth_buffer});
	frame_buffer = new FrameBuffer(render_pass, {tex, depth_buffer});
	dset_out->set_texture(0, tex);
	dset_out->update();
#endif
}

void LinearToSrgbRenderer::prepare(const RenderParams& p) {
	const auto area = p.area;
	ensure_fb_size(area);

	auto pp = p.with_target(frame_buffer.get());
#if HAS_LIB_VULKAN
	pp.render_pass = render_pass;
#endif

	for (auto c: children)
		c->prepare(pp);

#if HAS_LIB_GL
	nix::bind_frame_buffer(frame_buffer.get());
	nix::set_viewport(area);
	nix::clear_z();
	nix::set_projection_ortho_pixel();
	nix::set_z(true,true);

	for (auto c: children)
		c->draw(pp);
#endif
#if HAS_LIB_VULKAN
	auto cb = p.command_buffer;
	cb->begin_render_pass(render_pass, frame_buffer.get());
	cb->set_viewport(area);

	for (auto c: children)
		c->draw(pp);

	cb->end_render_pass();
#endif
}

void LinearToSrgbRenderer::draw(const RenderParams& p) {
	auto source = rect::ID;//p.area;
	if (source != vb_2d_current_source) {
		vb_2d->create_quad(rect::ID_SYM, source);
		vb_2d_current_source = source;
	}

#if HAS_LIB_GL
	auto ctx = session->ctx;
	const auto area = p.area;
	nix::set_shader(shader_out.get());
	//nix::vb_temp->create_quad(rect::ID_SYM, rect(0, area.width() / frame_buffer->width, 1 - area.height() / frame_buffer->height, 1));
	ctx->vb_temp->create_quad(rect::ID_SYM, rect(0, area.width() / frame_buffer->width, 1 - area.height() / frame_buffer->height, 1));
	nix::bind_texture(0, weak(frame_buffer->color_attachments)[0]);
	nix::set_z(false, false);
	nix::set_cull(nix::CullMode::NONE);
	nix::draw_triangles(session->ctx->vb_temp);

#endif
#if HAS_LIB_VULKAN
	auto cb = p.command_buffer;
	if (!pipeline_out) {
		pipeline_out = new vulkan::GraphicsPipeline(shader_out.get(), p.render_pass, 0, "triangles", "3f,3f,2f");
		pipeline_out->set_culling(CullMode::NONE);
		pipeline_out->set_z(false, false);
		pipeline_out->rebuild();
	}

	cb->bind_pipeline(pipeline_out);
	cb->bind_descriptor_set(0, dset_out);
	cb->draw(vb_2d);
#endif
}

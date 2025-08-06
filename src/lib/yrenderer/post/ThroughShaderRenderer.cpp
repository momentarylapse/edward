//
// Created by michi on 12/8/24.
//
#include "ThroughShaderRenderer.h"
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/profiler/Profiler.h>
#include <lib/math/mat4.h>

namespace yrenderer {

ThroughShaderRenderer::ThroughShaderRenderer(Context* ctx, const string& name, shared<ygfx::Shader> _shader) :
	Renderer(ctx, name),
	bindings(_shader.get())
{
	shader = _shader;
	vb_2d = new ygfx::VertexBuffer("3f,3f,2f");
	vb_2d->create_quad(rect::ID_SYM);
	current_area = rect::ID;
}

void ThroughShaderRenderer::set_source(const rect& area) {
	if (area == current_area)
		return;
	current_area = area;
	vb_2d->create_quad(rect::ID_SYM, area);
}

void ThroughShaderRenderer::draw(const RenderParams &params) {
#ifdef USING_VULKAN
	auto cb = params.command_buffer;

	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	if (!pipeline) {
		pipeline = new vulkan::GraphicsPipeline(shader.get(), params.render_pass, 0, ygfx::PrimitiveTopology::TRIANGLES, "3f,3f,2f");
		pipeline->set_culling(ygfx::CullMode::NONE);
		pipeline->set_z(false, false);
		pipeline->rebuild();
	}

	cb->bind_pipeline(pipeline);
	bindings.apply(shader.get(), params);
	cb->draw(vb_2d.get());


	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
#else
	bool flip_y = params.target_is_window;

	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	nix::set_shader(shader.get());
	bindings.apply(shader.get(), params);
	shader->set_matrix_l(shader->location[ygfx::Shader::LOCATION_MATRIX_P], flip_y ? mat4::scale(1,-1,1) : mat4::ID);
	shader->set_matrix_l(shader->location[ygfx::Shader::LOCATION_MATRIX_V], mat4::ID);
	shader->set_matrix_l(shader->location[ygfx::Shader::LOCATION_MATRIX_M], mat4::ID);
	nix::set_cull(nix::CullMode::NONE);

	nix::set_z(false, false);

	nix::draw_triangles(vb_2d.get());

	nix::set_cull(nix::CullMode::BACK);

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
#endif
}

}


//
// Created by michi on 12/8/24.
//
#include "ThroughShaderRenderer.h"
#include "../base.h"
#include "../../graphics-impl.h"
#include "../../helper/PerformanceMonitor.h"
#include <lib/math/mat4.h>


ThroughShaderRenderer::ThroughShaderRenderer(const string& name, shared<Shader> _shader) :
	Renderer(name),
	bindings(_shader.get())
{
	shader = _shader;
	vb_2d = new VertexBuffer("3f,3f,2f");
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

	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	if (!pipeline) {
		pipeline = new vulkan::GraphicsPipeline(shader.get(), params.render_pass, 0, "triangles", "3f,3f,2f");
		pipeline->set_culling(CullMode::NONE);
		pipeline->set_z(false, false);
		pipeline->rebuild();
	}

	cb->bind_pipeline(pipeline);
	bindings.apply(shader.get(), params);
	cb->draw(vb_2d.get());


	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
#else
	bool flip_y = params.target_is_window;

	PerformanceMonitor::begin(channel);
	gpu_timestamp_begin(params, channel);

	nix::set_shader(shader.get());
	bindings.apply(shader.get(), params);
	nix::set_projection_matrix(flip_y ? mat4::scale(1,-1,1) : mat4::ID);
	nix::set_view_matrix(mat4::ID);
	nix::set_model_matrix(mat4::ID);
	nix::set_cull(nix::CullMode::NONE);

	nix::set_z(false, false);

	nix::draw_triangles(vb_2d.get());

	nix::set_cull(nix::CullMode::BACK);

	gpu_timestamp_end(params, channel);
	PerformanceMonitor::end(channel);
#endif
}


#include "RenderViewData.h"
#include "../../../graphics-impl.h"
#ifdef USING_OPENGL
#include "GeometryRendererGL.h"


void RenderData::apply(const RenderParams &params) {
}

RenderViewData::RenderViewData() {
	ubo_light = new UniformBuffer();
}

void RenderViewData::begin_scene(SceneView *_scene_view) {
	scene_view = _scene_view;
	nix::bind_uniform_buffer(1, ubo_light.get());
}

void RenderViewData::set_projection_matrix(const mat4& projection) {
	nix::set_projection_matrix(projection);
}
void RenderViewData::set_view_matrix(const mat4& view) {
	nix::set_view_matrix(view);
}

RenderData& RenderViewData::start(const RenderParams& params, RenderPathType type, const mat4& matrix,
                                  ShaderCache& shader_cache, const Material& material, int pass_no,
                                  const string& vertex_shader_module, const string& geometry_shader_module,
                                  PrimitiveTopology top, VertexBuffer *vb) {
	nix::set_model_matrix(matrix);
	GeometryRendererGL::set_material(*scene_view, shader_cache, material, type, vertex_shader_module, geometry_shader_module);
	return rd;
}
#endif

#ifdef USING_VULKAN
#include "GeometryRendererVulkan.h"
#include "SceneView.h"
#include "../../base.h"

RenderViewData::RenderViewData() {
	ubo_light = new UniformBuffer(MAX_LIGHTS * sizeof(UBOLight));
}

void RenderViewData::begin_scene(SceneView *_scene_view) {
	scene_view = _scene_view;
	index = 0;
}

void RenderViewData::set_projection_matrix(const mat4& projection) {
	ubo.p = projection;
}
void RenderViewData::set_view_matrix(const mat4& view) {
	ubo.v = view;
}

RenderData& RenderViewData::start(
		const RenderParams& params, RenderPathType type, const mat4& matrix,
		ShaderCache& shader_cache, const Material& material, int pass_no,
		const string& vertex_shader_module, const string& geometry_shader_module,
		PrimitiveTopology top, VertexBuffer *vb) {
	shader_cache._prepare_shader_multi_pass(type, material, vertex_shader_module, geometry_shader_module, pass_no);
	if (index >= rda.num) {
		rda.add({new UniformBuffer(sizeof(UBO)),
		         pool->create_set(shader_cache.get_shader(type))});
		rda[index].dset->set_uniform_buffer(BINDING_PARAMS, rda[index].ubo);
		rda[index].dset->set_uniform_buffer(BINDING_LIGHT, ubo_light.get());
	}

	ubo.m = matrix;
	ubo.albedo = material.albedo;
	ubo.emission = material.emission;
	ubo.metal = material.metal;
	ubo.roughness = material.roughness;
	rda[index].ubo->update_part(&ubo, 0, sizeof(UBO));

	auto s = shader_cache.get_shader(type);
	auto p = GeometryRendererVulkan::get_pipeline(s, params.render_pass, material.pass(pass_no), top, vb);

	params.command_buffer->bind_pipeline(p);

	if (scene_view)
		rda[index].set_textures(*scene_view, weak(material.textures));

	return rda[index ++];
}

void RenderData::set_textures(const SceneView& scene_view, const Array<Texture*>& tex) {
	foreachi (auto t, tex, i)
						if (t)
							dset->set_texture(BINDING_TEX0 + i, t);
	if (scene_view.fb_shadow1)
		dset->set_texture(BINDING_SHADOW0, scene_view.fb_shadow1->attachments[1].get());
	if (scene_view.fb_shadow1)
		dset->set_texture(BINDING_SHADOW1, scene_view.fb_shadow2->attachments[1].get());
	if (scene_view.cube_map)
		dset->set_texture(BINDING_CUBE, scene_view.cube_map.get());
}

void RenderData::apply(const RenderParams& params) {
	dset->update();
	params.command_buffer->bind_descriptor_set(0, dset);
}

#endif

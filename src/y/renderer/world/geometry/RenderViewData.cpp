#include "RenderViewData.h"

#include <algorithm>
#include <lib/base/iter.h>
#include <renderer/path/RenderPath.h>

#include "../../../graphics-impl.h"
#include "GeometryRenderer.h"
#include "SceneView.h"
#include "../../base.h"
#ifdef USING_OPENGL
#include <world/Camera.h>
#include <y/Entity.h>
#endif

extern float global_shadow_box_size; // :(


RenderViewData::RenderViewData() {
	type = RenderPathType::Forward;
	ubo_light = new UniformBuffer(sizeof(LightMetaData) + MAX_LIGHTS * sizeof(UBOLight));
	light_meta_data = {};
	set_projection_matrix(mat4::ID);
	set_view_matrix(mat4::ID);
}

void RenderViewData::set_projection_matrix(const mat4& projection) {
	ubo.p = projection;
}
void RenderViewData::set_view_matrix(const mat4& view) {
	ubo.v = view;
}

void RenderViewData::update_lights() {
	Array<UBOLight> lights;
	light_meta_data.shadow_index = -1;
	for (auto l: scene_view->lights) {
		l->update(scene_view->cam, global_shadow_box_size, true);
		lights.add(l->light);
	}
	for (const auto& [i,l]: enumerate(scene_view->shadow_indices)) {
		light_meta_data.shadow_index = l;
		light_meta_data.shadow_proj[i] = scene_view->lights[i]->shadow_projection;
	}
	light_meta_data.num_lights = scene_view->lights.num;
	ubo_light->update_part(&light_meta_data, 0, sizeof(LightMetaData));
	ubo_light->update_array(lights, sizeof(LightMetaData));
}

void RenderViewData::prepare_scene(SceneView *_scene_view) {
	scene_view = _scene_view;
	update_lights();
}


#ifdef USING_OPENGL

void RenderData::set_material_x(const SceneView& scene_view, const Material& material, Shader* shader, int pass_no) {
	nix::set_shader(shader);
	if constexpr (GeometryRenderer::using_view_space)
		shader->set_floats("eye_pos", &scene_view.cam->owner->pos.x, 3); // NAH....
	else
		shader->set_floats("eye_pos", &vec3::ZERO.x, 3);
	for (auto &u: material.uniforms)
		shader->set_floats(u.name, u.p, u.size/4);

	auto& pass = material.pass(pass_no);
	nix::set_z(pass.z_buffer, pass.z_test);
	if (pass.mode == TransparencyMode::FUNCTIONS)
		nix::set_alpha(pass.source, pass.destination);
	else if (pass.mode == TransparencyMode::COLOR_KEY_HARD)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else if (pass.mode == TransparencyMode::MIX)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else
		nix::disable_alpha();

	nix::bind_textures(weak(material.textures));
	nix::bind_texture(BINDING_CUBE, scene_view.cube_map.get());

	shader->set_color_l(shader->location[Shader::LOCATION_MATERIAL_ALBEDO], material.albedo);
	shader->set_float_l(shader->location[Shader::LOCATION_MATERIAL_ROUGHNESS], material.roughness);
	shader->set_float_l(shader->location[Shader::LOCATION_MATERIAL_METAL], material.metal);
	shader->set_color_l(shader->location[Shader::LOCATION_MATERIAL_EMISSION], material.emission);
}

void RenderData::draw_triangles(const RenderParams&, VertexBuffer* vb) {
	nix::draw_triangles(vb);
}

void RenderData::draw_instanced(const RenderParams&, VertexBuffer* vb, int count) {
	nix::draw_instanced_triangles(vb, count);
}

void RenderData::draw(const RenderParams& params, VertexBuffer* vb, PrimitiveTopology topology) {
	if (topology == PrimitiveTopology::TRIANGLES)
		nix::draw_triangles(vb);
	else if (topology == PrimitiveTopology::POINTS)
		nix::draw_points(vb);
	else if (topology == PrimitiveTopology::LINES)
		nix::draw_lines(vb, false);
	else if (topology == PrimitiveTopology::LINE_STRIP)
		nix::draw_lines(vb, true);
}


void RenderViewData::begin_draw() {
	nix::bind_uniform_buffer(BINDING_LIGHT, ubo_light.get());
}

void RenderViewData::set_z(bool write, bool test) {
	nix::set_z(write, test);
}
void RenderViewData::set_wire(bool enabled) {
	nix::set_wire(enabled);
}
void RenderViewData::set_cull(CullMode mode) {
	nix::set_cull(mode);
}

RenderData& RenderViewData::start(const RenderParams& params, const mat4& matrix,
                                  Shader* shader, const Material& material, int pass_no,
                                  PrimitiveTopology top, VertexBuffer *vb) {

	rd.set_material_x(*scene_view, material, shader, pass_no);

	//shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_MVP], ubo.p * ubo.v * matrix);
	shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_M], matrix);
	shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_V], ubo.v);
	shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_P], ubo.p);

	return rd;
}

#endif

#ifdef USING_VULKAN

void RenderViewData::begin_draw() {
	index = 0;
	light_meta_data.num_surfels = 0;
	if (scene_view)
		light_meta_data.num_surfels = scene_view->num_surfels;
}

RenderData& RenderViewData::start(
		const RenderParams& params, const mat4& matrix,
		Shader* shader, const Material& material, int pass_no,
		PrimitiveTopology top, VertexBuffer *vb) {
	if (index >= rda.num) {
		rda.add({new UniformBuffer(sizeof(UBO)),
		         pool->create_set(shader)});
		rda[index].dset->set_uniform_buffer(BINDING_PARAMS, rda[index].ubo);
		rda[index].dset->set_uniform_buffer(BINDING_LIGHT, ubo_light.get());
	}

	ubo.m = matrix;
	ubo.albedo = material.albedo;
	ubo.emission = material.emission;
	ubo.metal = material.metal;
	ubo.roughness = material.roughness;
	rda[index].ubo->update_part(&ubo, 0, sizeof(UBO));

	auto p = GeometryRenderer::get_pipeline(shader, params.render_pass, material.pass(pass_no), top, vb);

	params.command_buffer->bind_pipeline(p);

	if (scene_view) {
		rda[index].set_textures(*scene_view, weak(material.textures));
		if (scene_view->surfel_buffer)
			rda[index].dset->set_uniform_buffer(12, scene_view->surfel_buffer.get());
	}

	return rda[index ++];
}

void RenderData::set_textures(const SceneView& scene_view, const Array<Texture*>& tex) {
	foreachi (auto t, tex, i)
						if (t)
							dset->set_texture(BINDING_TEX0 + i, t);
	if (scene_view.shadow_maps.num >= 1)
		dset->set_texture(BINDING_SHADOW0, scene_view.shadow_maps[0]);
	if (scene_view.shadow_maps.num >= 2)
		dset->set_texture(BINDING_SHADOW1, scene_view.shadow_maps[1]);
	if (scene_view.cube_map)
		dset->set_texture(BINDING_CUBE, scene_view.cube_map.get());
}

void RenderData::draw_triangles(const RenderParams& params, VertexBuffer* vb) {
	dset->update();
	params.command_buffer->bind_descriptor_set(0, dset);
	params.command_buffer->draw(vb);
}

void RenderData::draw_instanced(const RenderParams& params, VertexBuffer* vb, int count) {
	dset->update();
	params.command_buffer->bind_descriptor_set(0, dset);
	params.command_buffer->draw_instanced(vb, count);
}

void RenderData::draw(const RenderParams& params, VertexBuffer* vb, PrimitiveTopology topology) {
	// topology defined by pipeline...
	draw_triangles(params, vb);
}

#endif



Shader* RenderViewData::get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module) {
	if (!multi_pass_shader_cache[pass_no].contains(material))
		multi_pass_shader_cache[pass_no].set(material, {});
	auto& cache = multi_pass_shader_cache[pass_no][material];
	if (is_shadow_pass())
		cache._prepare_shader_multi_pass(type, *material_shadow, vertex_shader_module, geometry_shader_module, pass_no);
	else
		cache._prepare_shader_multi_pass(type, *material, vertex_shader_module, geometry_shader_module, pass_no);
	return cache.get_shader(type);
}

bool RenderViewData::is_shadow_pass() const {
	return material_shadow;
}



#include "RenderViewData.h"
#include "SceneRenderer.h"
#include "CameraParams.h"
#include "SceneView.h"
#include "Light.h"

#include <algorithm>
#include <lib/base/iter.h>
#include <lib/os/msg.h>

#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/Bindable.h>

namespace yrenderer {
using namespace ygfx;


RenderViewData::RenderViewData(Context* _ctx) {
	ctx = _ctx;
	type = RenderPathType::Forward;
	ubo_light = new UniformBuffer(sizeof(LightMetaData) + MAX_LIGHTS * sizeof(UBOLight));
	light_meta_data = {};
	set_view(RenderParams::WHATEVER, CameraParams{});
}

void RenderViewData::set_scene_view(SceneView* _scene_view) {
	scene_view = _scene_view;
}

void RenderViewData::set_view(const RenderParams& params, const CameraParams& view, const mat4* proj) {
	bool flip_y = params.target_is_window;

#ifdef USING_OPENGL
	auto m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
#else
	auto m = mat4::ID;
#endif

	camera_params = view;
	ubo.v = view.view_matrix();
	if (proj)
		ubo.p = *proj * m;
	else
		ubo.p = view.projection_matrix(params.desired_aspect_ratio) * m;
}


void RenderViewData::update_light_ubo() {
	Array<UBOLight> lights;
	lights.resize(scene_view->lights.num);
	for (auto&& [i, l]: enumerate(scene_view->lights))
		// using current view
		lights[i] = l->to_ubo(camera_params.pos, camera_params.ang, true);

	for (const auto [i,l]: enumerate(scene_view->shadow_indices)) {
		auto ll = scene_view->lights[l];
		// from reference cam
		ll->shadow_projection = ll->suggest_shadow_projection(scene_view->main_camera_params, scene_view->shadow_box_size);
		if constexpr (true)
			light_meta_data.shadow_proj[ll->shadow_index] = ll->shadow_projection * ubo.v.inverse();
		else
			light_meta_data.shadow_proj[ll->shadow_index] = ll->shadow_projection;
	}
	light_meta_data.num_lights = scene_view->lights.num;
	ubo_light->update_part(&light_meta_data, 0, sizeof(LightMetaData));
	ubo_light->update_array(lights, sizeof(LightMetaData));
}


#ifdef USING_OPENGL

void RenderData::set_material_x(const SceneView& scene_view, const Material& material, Shader* shader, int pass_no) {
	nix::set_shader(shader);
	if constexpr (SceneRenderer::using_view_space)
		shader->set_floats("eye_pos", &scene_view.main_camera_params.pos.x, 3); // NAH....
	else
		shader->set_floats("eye_pos", &vec3::ZERO.x, 3);
	apply_shader_data(RenderParams{}, shader, material.shader_data);

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

void RenderData::set_texture(int binding, Texture *tex) {
	nix::bind_texture(binding, tex);
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

void RenderViewData::clear(const RenderParams& params, const Array<color>& colors, float z) {
	if (colors.num > 0)
		nix::clear_color(colors[0]);
	if (z >= 0)
		nix::clear_z();
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
	if (scene_view) {
		light_meta_data.num_surfels = scene_view->num_surfels;
		light_meta_data.probe_cells = scene_view->probe_cells;
		light_meta_data.probe_min = scene_view->probe_min;
		light_meta_data.probe_max = scene_view->probe_max;
	} else {
		light_meta_data.num_surfels = 0;
		light_meta_data.probe_cells = ivec3{0, 0, 0};
	}
}

void RenderViewData::clear(const RenderParams& params, const Array<color>& colors, float z) {
	auto cb = params.command_buffer;
	if (z >= 0)
		cb->clear(params.area, colors, z);
	else
		cb->clear(params.area, colors, base::None);
}


RenderData& RenderViewData::start(
		const RenderParams& params, const mat4& matrix,
		Shader* shader, const Material& material, int pass_no,
		PrimitiveTopology top, VertexBuffer *vb) {
	if (index >= rda.num) {
		rda.add({new UniformBuffer(sizeof(UBO)),
		         ctx->pool->create_set(shader)});
		rda[index].dset->set_uniform_buffer(BINDING_PARAMS, rda[index].ubo);
		rda[index].dset->set_uniform_buffer(BINDING_LIGHT, ubo_light.get());
	}

	ubo.m = matrix;
	ubo.albedo = material.albedo;
	ubo.emission = material.emission;
	ubo.metal = material.metal;
	ubo.roughness = material.roughness;
	rda[index].ubo->update_part(&ubo, 0, sizeof(UBO));

	auto p = SceneRenderer::get_pipeline(shader, params.render_pass, material.pass(pass_no), top, vb);

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

void RenderData::set_texture(int binding, Texture *tex) {
	dset->set_texture(binding, tex);
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

}



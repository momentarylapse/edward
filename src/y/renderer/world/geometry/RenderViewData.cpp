#include "RenderViewData.h"
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
	ubo_light = new UniformBuffer(MAX_LIGHTS * sizeof(UBOLight));
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
	for (auto l: scene_view->lights) {
		l->update(scene_view->cam, global_shadow_box_size, true);
		if (l->allow_shadow) {
			shadow_proj = l->shadow_projection;
		}
		lights.add(l->light);
	}
	ubo_light->update_array(lights);
	//ubo_light->update_part(&lights[0], 0, lights.num * sizeof(lights[0]));
}

void RenderViewData::prepare_scene(SceneView *_scene_view) {
	scene_view = _scene_view;
	update_lights();
}


#ifdef USING_OPENGL

void RenderData::apply(const RenderParams &params) {
}

void RenderViewData::begin_draw() {
	nix::set_projection_matrix(ubo.p);
	nix::set_view_matrix(ubo.v);
	nix::bind_uniform_buffer(1, ubo_light.get());
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
	nix::set_model_matrix(matrix);

	nix::set_shader(shader);
	if (GeometryRenderer::using_view_space)
		shader->set_floats("eye_pos", &scene_view->cam->owner->pos.x, 3); // NAH....
	else
		shader->set_floats("eye_pos", &vec3::ZERO.x, 3);
	shader->set_int("num_lights", scene_view->lights.num);
	shader->set_int("shadow_index", scene_view->shadow_index);
	for (auto &u: material.uniforms)
		shader->set_floats(u.name, u.p, u.size/4);

	auto& pass = material.pass(pass_no);
	if (pass.mode == TransparencyMode::FUNCTIONS)
		nix::set_alpha(pass.source, pass.destination);
	else if (pass.mode == TransparencyMode::COLOR_KEY_HARD)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else if (pass.mode == TransparencyMode::MIX)
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	else
		nix::disable_alpha();

	nix::bind_textures(weak(material.textures));
	nix::bind_texture(7, scene_view->cube_map.get());


	nix::set_material(material.albedo, material.roughness, material.metal, material.emission);

	return rd;
}
#endif

#ifdef USING_VULKAN

void RenderViewData::begin_draw() {
	index = 0;
	ubo.num_surfels = 0;
	if (scene_view)
		ubo.num_surfels = scene_view->num_surfels;
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

void RenderData::apply(const RenderParams& params) {
	dset->update();
	params.command_buffer->bind_descriptor_set(0, dset);
}

#endif

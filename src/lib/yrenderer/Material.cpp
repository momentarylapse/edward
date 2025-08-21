#include "Material.h"
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include "ShaderManager.h"

#include "Context.h"

namespace yrenderer {

	using namespace ygfx;


Material::Material(Context* _ctx) {
	ctx = _ctx;
	// default values
	reflection.cube_map = nullptr;

	albedo = White;
	roughness = 0.6f;
	metal = 0.0f;
	emission = Black;

	pass0.cull_mode = CullMode::BACK;

	cast_shadow = true;

	reflection.mode = ReflectionMode::NONE;
	reflection.density = 0;
	reflection.cube_map_size = 0;

	friction.jump = 0.5f;
	friction._static = 0.8f;
	friction.sliding = 0.4f;
	friction.rolling = 0.90f;
}

xfer<Material> Material::copy() {
	auto m = new Material(ctx);
	m->albedo = albedo;
	m->roughness = roughness;
	m->metal = metal;
	m->emission = emission;

	m->textures = textures;
	m->cast_shadow = cast_shadow;

	m->pass0 = pass0;
	m->num_passes = num_passes;
	if (extended) {
		m->extended = new ExtendedData;
		*m->extended = *extended;
	}
	m->reflection = reflection;
	m->reflection.cube_map = reflection.cube_map;
/*	if ((cube_map < 0) and (m2->cube_map_size > 0) and (reflection.mode == ReflectionCubeMapDynamical)){
		cube_map = FxCubeMapNew(m2->cube_map_size);
		FxCubeMapCreate(cube_map, model);
	}*/
	m->friction = friction;
	return m;
}

inline int shader_index(RenderPathType render_path_type) {
	return (int)render_path_type - 1;
}


void ShaderCache::_prepare_shader(RenderPathType render_path_type, const Material &material, const string& vertex_module, const string& geometry_module) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = material.ctx->shader_manager->load_surface_shader(material.pass0.shader_path, rpt, vertex_module, geometry_module);
}
void ShaderCache::_prepare_shader_multi_pass(RenderPathType render_path_type, const Material &material, const string& vertex_module, const string& geometry_module, int k) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = material.ctx->shader_manager->load_surface_shader(material.pass(k).shader_path, rpt, vertex_module, geometry_module);
}

Shader *ShaderCache::get_shader(RenderPathType render_path_type) {
	int i = shader_index(render_path_type);
	//_prepare_shader(render_path_type, v);
	return shader[i].get();
}


const Material::RenderPassData& Material::pass(int k) const {
	if (k == 0)
		return pass0;
	return extended->pass[k - 1];
}

Material::RenderPassData& Material::pass(int k) {
	if (k == 0)
		return pass0;
	return extended->pass[k - 1];
}

bool Material::is_transparent() const {
	if (extended)
		return true;
	//!alpha.z_buffer; //false;
	if (pass0.mode == TransparencyMode::FUNCTIONS)
		return true;
	if (pass0.mode == TransparencyMode::FACTOR)
		return true;
	if (pass0.mode == TransparencyMode::MIX)
		return true;
	return false;
}

}

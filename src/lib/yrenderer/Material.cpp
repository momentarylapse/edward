#include "Material.h"
#include <lib/os/msg.h>
#include <lib/ygraphics/graphics-impl.h>
#include "ShaderManager.h"

#include "Context.h"

namespace yrenderer {

using namespace ygfx;

Material::RenderPassData::RenderPassData() {
	cull_mode = CullMode::BACK;
	source = Alpha::ONE;
	destination = Alpha::ZERO;
}

Material::Material() {
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

Material::Material(const Material &m) {
	*this = m;
}

void Material::operator=(const Material& o) {
	parent = o.parent;

	albedo = o.albedo;
	roughness = o.roughness;
	metal = o.metal;
	emission = o.emission;

	textures = o.textures;
	cast_shadow = o.cast_shadow;

	set_num_passes(o.num_passes);
	for (int k=0; k<o.num_passes; k++)
		pass(k) = o.pass(k);

	reflection = o.reflection;
	reflection.cube_map = o.reflection.cube_map;
	/*	if ((cube_map < 0) and (m2->cube_map_size > 0) and (reflection.mode == ReflectionCubeMapDynamical)){
			cube_map = FxCubeMapNew(m2->cube_map_size);
			FxCubeMapCreate(cube_map, model);
		}*/
	friction = o.friction;
}

void Material::set_num_passes(int _num_passes) {
	num_passes = _num_passes;
	if (num_passes >= 2 and !extended)
		extended = new ExtendedData;
}

xfer<Material> Material::copy() const {
	auto m = new Material();
	*m = *this;
	return m;
}

void Material::derive_from(Material* _parent) {
	if (_parent)
		*this = *_parent;
	parent = _parent;
}

inline int shader_index(RenderPathType render_path_type) {
	return (int)render_path_type - 1;
}


void ShaderCache::_prepare_shader(RenderPathType render_path_type, const Material* material, const string& vertex_module, const string& geometry_module) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = ctx->shader_manager->load_surface_shader(material->pass0.shader_path, rpt, vertex_module, geometry_module);
}
void ShaderCache::_prepare_shader_multi_pass(RenderPathType render_path_type, const Material* material, const string& vertex_module, const string& geometry_module, int k) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = ctx->shader_manager->load_surface_shader(material->pass(k).shader_path, rpt, vertex_module, geometry_module);
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

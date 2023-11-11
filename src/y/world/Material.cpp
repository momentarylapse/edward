#include "Material.h"
#include "Model.h"
#include <lib/config.h>
#include <lib/os/msg.h>
#include <lib/os/config.h>
#include <lib/any/any.h>
#include "../graphics-impl.h"
#include "../helper/ResourceManager.h"
#include "../y/EngineData.h"



MaterialManager::MaterialManager(ResourceManager *_resource_manager) {
	resource_manager = _resource_manager;
	// create the default material
	trivial_material = new Material(resource_manager);
	//trivial_material->shader_path = Shader::default_3d;

	set_default(trivial_material);
}

MaterialManager::~MaterialManager() {
	delete trivial_material;
}

void MaterialManager::reset() {
	for (auto&& [f, m]: materials)
		delete m;
	materials.clear();

	set_default(trivial_material);
}


void MaterialManager::set_default(Material *m) {
	default_material = m;
}

/*void MaterialManager::set_default_shader(Shader *s) {
	default_material->shader[0] = s;
	Shader::default_load = s;
}*/



Material::Material(ResourceManager *rm) {
	resource_manager = rm;
	// default values
	reflection.cube_map = nullptr;

	albedo = White;
	roughness = 0.6f;
	metal = 0.0f;
	emission = Black;

	cast_shadow = true;

	reflection.mode = ReflectionMode::NONE;
	reflection.density = 0;
	reflection.cube_map_size = 0;

	friction.jump = 0.5f;
	friction._static = 0.8f;
	friction.sliding = 0.4f;
	friction.rolling = 0.90f;
}

void Material::add_uniform(const string &name, float *p, int size) {
	uniforms.add({name, p, size});
}

xfer<Material> Material::copy() {
	auto m = new Material(resource_manager);
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

float col_frac(const color &a, const color &b) {
	return (a.r+a.g+a.b) / (b.r+b.g+b.b);
}

static const Alpha FILE_ALPHAS[] = {
		Alpha::ZERO,
		Alpha::ONE,
		Alpha::SOURCE_COLOR,
		Alpha::SOURCE_INV_COLOR,
		Alpha::SOURCE_ALPHA,
		Alpha::SOURCE_INV_ALPHA,
		Alpha::DEST_COLOR,
		Alpha::DEST_INV_COLOR,
		Alpha::DEST_ALPHA,
		Alpha::DEST_INV_ALPHA};

Alpha parse_alpha_i(int a) {
	return FILE_ALPHAS[clamp(a, 0, 10)];
}

Alpha parse_alpha(const string& s) {
	if (s == "zero")
		return Alpha::ZERO;
	if (s == "one")
		return Alpha::ONE;
	if (s == "source-color")
		return Alpha::SOURCE_COLOR;
	if (s == "source-inv-color")
		return Alpha::SOURCE_INV_COLOR;
	if (s == "source-alpha")
		return Alpha::SOURCE_ALPHA;
	if (s == "source-inv-alpha")
		return Alpha::SOURCE_INV_ALPHA;
	if (s == "dest-color")
		return Alpha::DEST_COLOR;
	if (s == "dest-inv-color")
		return Alpha::DEST_INV_COLOR;
	if (s == "dest-alpha")
		return Alpha::DEST_ALPHA;
	if (s == "dest-inv-alpha")
		return Alpha::DEST_INV_ALPHA;
	return parse_alpha_i(s._int());
}

color any2color(const Any &a) {
	if (a.is_string())
		return color::parse(a.str());
	if (a.is_array() and (a.length() >= 3)) {
		color c = White;
		c.r = a[0]._float();
		c.g = a[1]._float();
		c.b = a[2]._float();
		if (a.length() >= 4)
			c.a = a[3]._float();
		return c;
	}
	return Black;
}


xfer<Material> MaterialManager::load(const Path &filename) {
	// an empty name loads the default material
	if (filename.is_empty())
		return default_material->copy();

	for (auto&& [f, m]: materials)
		if (f == filename)
			return m->copy();


	msg_write("loading material " + filename.str());

	Configuration c;
	if (!c.load(engine.material_dir | filename.with(".material"))) {
		/*if (engine.ignore_missing_files) {
			msg_error("material file missing: " + filename.str());
			return default_material->copy();
		} else {
			throw Exception("material file missing: " + filename.str());
		}*/
	}
	Material *m = new Material(resource_manager);

	m->albedo = any2color(c.get("color.albedo"));
	m->roughness = c.get_float("color.roughness", 0.5f);
	m->metal = c.get_float("color.metal", 0.1f);
	m->emission = any2color(c.get("color.emission"));

	auto texture_files = c.get_str_array("textures");
	for (auto &f: texture_files)
		m->textures.add(resource_manager->load_texture(f));
	m->pass0.shader_path = c.get_str("shader", "");
	m->cast_shadow = c.get_bool("shadow.cast", true);

	m->friction._static = c.get_float("friction.static", 0.5f);
	m->friction.sliding = c.get_float("friction.slide", 0.5f);
	m->friction.rolling = c.get_float("friction.roll", 0.5f);
	m->friction.jump = c.get_float("friction.jump", 0.5f);

	auto add_pass = [m] (int index) -> Material::RenderPassData& {
		if (index == 1)
			m->extended = new Material::ExtendedData;
		m->num_passes = max(m->num_passes, index + 1);
		return m->pass(index);
	};
	auto try_parse_pass = [m, &c, add_pass] (const string& key, int index) {
		if (!c.has(key + ".mode"))
			return;
		auto& p = add_pass(index);
		p.shader_path = c.get_str(key + ".shader", "");
		string mode = c.get_str(key + ".cull", "ccw");
		p.cull_mode = 1;
		if (mode == "none")
			p.cull_mode = 0;
		else if (mode == "cw" or mode == "front")
			p.cull_mode = 2;

		mode = c.get_str(key + ".mode", "solid");
		if (mode == "solid" or mode == "none") {
			p.mode = TransparencyMode::NONE;
		} else if (mode == "factor") {
				p.mode = TransparencyMode::FACTOR;
			p.factor = c.get_float(key + ".factor");
			p.z_buffer = false;
		} else if (mode == "function") {
			p.mode = TransparencyMode::FUNCTIONS;
			p.source = parse_alpha(c.get_str(key + ".source", "zero"));
			p.destination = parse_alpha(c.get_str(key + ".dest", "zero"));
			p.z_buffer = false;
		} else if (mode == "key-hard") {
			p.mode = TransparencyMode::COLOR_KEY_HARD;
		} else if (mode == "key-smooth") {
			p.mode = TransparencyMode::COLOR_KEY_SMOOTH;
		} else if (mode == "mix") {
			p.mode = TransparencyMode::MIX;
		} else if (mode != "") {
			msg_error("unknown transparency mode: " + mode);
		}
	};

	try_parse_pass("transparency", 0);

	for (int i=0; i<4; i++)
		try_parse_pass(format("pass%d", i), i);

	string mode = c.get_str("reflection.mode", "");
	if (mode == "static") {
		m->reflection.mode = ReflectionMode::CUBE_MAP_STATIC;
		texture_files = c.get_str_array("reflection.cubemap");
		shared_array<Texture> cmt;
		for (auto &f: texture_files)
			cmt.add(resource_manager->load_texture(f));
		m->reflection.density = c.get_float("reflection.density", 1);
#if 0
			m->reflection.cube_map = new CubeMap(m->reflection.cube_map_size);
			for (int i=0;i<6;i++)
				m->reflection.cube_map->fill_side(i, cmt[i]);
#endif
	} else if (mode == "dynamic") {
		m->reflection.mode = ReflectionMode::CUBE_MAP_DYNAMIC;
		m->reflection.density = c.get_float("reflection.density", 1);
		m->reflection.cube_map_size = c.get_float("reflection.size", 128);
		//m->cube_map = FxCubeMapNew(m->cube_map_size);
		//FxCubeMapCreate(m->cube_map,cmt[0],cmt[1],cmt[2],cmt[3],cmt[4],cmt[5]);
	} else if (mode == "mirror") {
		m->reflection.mode = ReflectionMode::MIRROR;
	} else if (mode != "") {
		msg_error("unknown reflection mode: " + mode);
	}

	materials.set(filename, m);
	return m->copy();
}

inline int shader_index(RenderPathType render_path_type) {
	return (int)render_path_type - 1;
}


void ShaderCache::_prepare_shader(RenderPathType render_path_type, Material *material, const string& vertex_module, const string& geometry_module) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = material->resource_manager->load_surface_shader(material->pass0.shader_path, rpt, vertex_module, geometry_module);
}
void ShaderCache::_prepare_shader_multi_pass(RenderPathType render_path_type, Material *material, const string& vertex_module, const string& geometry_module, int k) {
	int i = shader_index(render_path_type);
	if (shader[i])
		return;
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = material->resource_manager->load_surface_shader(material->pass(k).shader_path, rpt, vertex_module, geometry_module);
}

Shader *ShaderCache::get_shader(RenderPathType render_path_type) {
	int i = shader_index(render_path_type);
	//_prepare_shader(render_path_type, v);
	return shader[i].get();
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

#include "Material.h"
#include "Model.h"
#include "../lib/config.h"
#include "../lib/os/msg.h"
#include "../lib/os/config.h"
#include "../lib/any/any.h"
#include "../graphics-impl.h"
#if __has_include("ResourceManager.h")
	#include "ResourceManager.h"
	#include "EngineData.h"
#else
	#include "../helper/ResourceManager.h"
	#include "../y/EngineData.h"
#endif



MaterialManager::MaterialManager(ResourceManager *_resource_manager) {
	resource_manager = _resource_manager;
	// create the default material
	trivial_material = new Material(resource_manager);
	trivial_material->name = "-default-";
	//trivial_material->shader_path = Shader::default_3d;

	set_default(trivial_material);
}

MaterialManager::~MaterialManager() {
	delete trivial_material;
}

void MaterialManager::reset() {
	for (auto *m: materials)
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

	alpha.mode = TransparencyMode::NONE;
	alpha.source = Alpha::ZERO;
	alpha.destination = Alpha::ZERO;
	alpha.factor = 1;
	alpha.z_buffer = true;

	reflection.mode = ReflectionMode::NONE;
	reflection.density = 0;
	reflection.cube_map_size = 0;

	friction.jump = 0.5f;
	friction._static = 0.8f;
	friction.sliding = 0.4f;
	friction.rolling = 0.90f;
}

Material::~Material() {
}

void Material::add_uniform(const string &name, float *p, int size) {
	uniforms.add({name, p, size});
}

Material* Material::copy() {
	auto m = new Material(resource_manager);
	m->name = name;
	m->albedo = albedo;
	m->roughness = roughness;
	m->metal = metal;
	m->emission = emission;

	m->textures = textures;
	m->cast_shadow = cast_shadow;

	m->alpha = alpha;
	m->reflection = reflection;
	m->reflection.cube_map = reflection.cube_map;
/*	if ((cube_map < 0) and (m2->cube_map_size > 0) and (reflection.mode == ReflectionCubeMapDynamical)){
		cube_map = FxCubeMapNew(m2->cube_map_size);
		FxCubeMapCreate(cube_map, model);
	}*/
	for (int i=0; i<3; i++)
		m->shader[i] = shader[i];
	m->shader_path = shader_path;
	m->friction = friction;
	return m;
}

float col_frac(const color &a, const color &b) {
	return (a.r+a.g+a.b) / (b.r+b.g+b.b);
}

static const Alpha FILE_ALPHAS[] = {Alpha::ZERO, Alpha::ONE, Alpha::SOURCE_COLOR, Alpha::SOURCE_INV_COLOR, Alpha::SOURCE_ALPHA, Alpha::SOURCE_INV_ALPHA, Alpha::DEST_COLOR, Alpha::DEST_INV_COLOR, Alpha::DEST_ALPHA, Alpha::DEST_INV_ALPHA};

Alpha parse_alpha(int a) {
	return FILE_ALPHAS[clamp(a, 0, 10)];
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


Material *MaterialManager::load(const Path &filename) {
	// an empty name loads the default material
	if (filename.is_empty())
		return default_material->copy();

	for (Material *m: materials)
		if (m->name == filename)
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
	m->name = filename;

	m->albedo = any2color(c.get("color.albedo"));
	m->roughness = c.get_float("color.roughness", 0.5f);
	m->metal = c.get_float("color.metal", 0.1f);
	m->emission = any2color(c.get("color.emission"));

	auto texture_files = c.get_str_array("textures");
	for (auto &f: texture_files)
		m->textures.add(resource_manager->load_texture(f));
	m->shader_path = c.get_str("shader", "");
	m->cast_shadow = c.get_bool("shadow.cast", true);

	m->friction._static = c.get_float("friction.static", 0.5f);
	m->friction.sliding = c.get_float("friction.slide", 0.5f);
	m->friction.rolling = c.get_float("friction.roll", 0.5f);
	m->friction.jump = c.get_float("friction.jump", 0.5f);

	string mode = c.get_str("transparency.mode", "");
	if (mode == "factor") {
		m->alpha.mode = TransparencyMode::FACTOR;
		m->alpha.factor = c.get_float("transparency.factor");
		m->alpha.z_buffer = false;
	} else if (mode == "function") {
		m->alpha.mode = TransparencyMode::FUNCTIONS;
		m->alpha.source = parse_alpha(c.get_int("transparency.source", 0));
		m->alpha.destination = parse_alpha(c.get_int("transparency.dest", 0));
		m->alpha.z_buffer = false;
	} else if (mode == "key-hard") {
		m->alpha.mode = TransparencyMode::COLOR_KEY_HARD;
	} else if (mode == "key-smooth") {
		m->alpha.mode = TransparencyMode::COLOR_KEY_SMOOTH;
	} else if (mode != "") {
		msg_error("unknown transparency mode: " + mode);
	}

	mode = c.get_str("reflection.mode", "");

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

	materials.add(m);
	return m->copy();
}

inline int shader_index(RenderPathType render_path_type, ShaderVariant v) {
	return (int)v + (int)ShaderVariant::_NUM * ((int)render_path_type - 1);
}

void Material::_prepare_shader(RenderPathType render_path_type, ShaderVariant v) {
	int i = shader_index(render_path_type, v);
	if (shader[i])
		return;
	static const string VARIANT_NAME[5] = {"default", "animated", "instanced", "lines", "points"};
	const string &vv = VARIANT_NAME[(int)v];
	static const string GEOMETRY_NAME[5] = {"", "", "", "lines", "points"};
	const string &gg = GEOMETRY_NAME[(int)v];
	static const string RENDER_PATH_NAME[3] = {"", "forward", "deferred"};
	const string &rpt = RENDER_PATH_NAME[(int)render_path_type];
	shader[i] = resource_manager->load_surface_shader(shader_path, rpt, vv, gg);
}

Shader *Material::get_shader(RenderPathType render_path_type, ShaderVariant v) {
	int i = shader_index(render_path_type, v);
	_prepare_shader(render_path_type, v);
	return shader[i].get();
}

bool Material::is_transparent() const {
	//!alpha.z_buffer; //false;
	if (alpha.mode == TransparencyMode::FUNCTIONS)
		return true;
	if (alpha.mode == TransparencyMode::FACTOR)
		return true;
	return false;
}

#include "Material.h"
#include "Model.h"
#include "../lib/file/file.h"
#include "../lib/nix/nix.h"
#include "../lib/config.h"
#ifdef _X_USE_HUI_
	#include "../lib/hui/hui.h"
	#include "ResourceManager.h"
	#include "EngineData.h"
#elif defined(_X_USE_HUI_MINIMAL_)
	#include "../lib/hui_minimal/hui.h"
	#include "../helper/ResourceManager.h"
	#include "../y/EngineData.h"
#endif


// materials
static Material *default_material;
static Material *trivial_material;
static Array<Material*> materials; // "originals"

void MaterialInit() {
	// create the default material
	trivial_material = new Material;
	trivial_material->name = "-default-";
	trivial_material->shader = nix::Shader::default_3d;

	SetDefaultMaterial(trivial_material);
}

void MaterialEnd() {
	delete trivial_material;
}

void MaterialReset() {
	for (auto *m: materials)
		delete m;
	materials.clear();

	SetDefaultMaterial(trivial_material);
}


void SetDefaultMaterial(Material *m) {
	default_material = m;
}

void MaterialSetDefaultShader(nix::Shader *s) {
	default_material->shader = s;
	nix::Shader::default_load = s;
}



Material::Material() {
	// default values
	reflection.cube_map = NULL;
	shader = NULL;

	albedo = White;
	roughness = 0.6f;
	metal = 0.0f;
	emission = Black;

	cast_shadow = true;

	alpha.mode = TransparencyMode::NONE;
	alpha.source = nix::Alpha::ZERO;
	alpha.destination = nix::Alpha::ZERO;
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
	int loc = shader->get_location(name);
	uniforms.add({loc, p, size});
}

Material* Material::copy() {
	Material *m = new Material;
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
	m->shader = shader;
	m->friction = friction;
	return m;
}

float col_frac(const color &a, const color &b) {
	return (a.r+a.g+a.b) / (b.r+b.g+b.b);
}


Material *LoadMaterial(const Path &filename) {
	// an empty name loads the default material
	if (filename.is_empty())
		return default_material->copy();

	for (Material *m: materials)
		if (m->name == filename)
			return m->copy();


	msg_write("loading material " + filename.str());

	hui::Configuration c;
	if (!c.load(engine.material_dir << filename.with(".material"))) {
		/*if (engine.ignore_missing_files) {
			msg_error("material file missing: " + filename.str());
			return default_material->copy();
		} else {
			throw Exception("material file missing: " + filename.str());
		}*/
	}
	Material *m = new Material;

	m->albedo = color::parse(c.get_str("color.albedo", ""));
	m->roughness = c.get_float("color.roughness", 0.5f);
	m->metal = c.get_float("color.metal", 0.1f);
	m->emission = color::parse(c.get_str("color.emission", ""));

	auto texture_files = c.get_str("textures", "");
	if (texture_files != "")
		for (auto &f: texture_files.explode(","))
			m->textures.add(ResourceManager::load_texture(f));
	m->shader = ResourceManager::load_shader(c.get_str("shader", ""));

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
		m->alpha.source = (nix::Alpha)c.get_int("transparency.source", 0);
		m->alpha.destination = (nix::Alpha)c.get_int("transparency.dest", 0);
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
		texture_files = c.get_str("reflection.cubemap", "");
		Array<nix::Texture*> cmt;
		for (auto &f: texture_files.explode(","))
			cmt.add(ResourceManager::load_texture(f));
		m->reflection.density = c.get_float("reflection.density", 1);
#if 0
			m->reflection.cube_map = new nix::CubeMap(m->reflection.cube_map_size);
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

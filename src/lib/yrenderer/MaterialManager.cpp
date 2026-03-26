//
// Created by michi on 8/4/25.
//

#include "MaterialManager.h"
#include "Context.h"
#include "TextureManager.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/any/any.h>
#include <lib/os/msg.h>
#include <lib/os/config.h>
#include <lib/base/iter.h>

namespace yrenderer {

	using namespace ygfx;

MaterialManager::MaterialManager(TextureManager* tm, const Path& _material_dir) : Node() {
	texture_manager = tm;
	material_dir = _material_dir;
	// create the default material
	trivial_material = create_internal();

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
	if (a.is_list() and (a.length() >= 3)) {
		color c = White;
		c.r = a[0].to_f32();
		c.g = a[1].to_f32();
		c.b = a[2].to_f32();
		if (a.length() >= 4)
			c.a = a[3].to_f32();
		return c;
	}
	return Black;
}

Path MaterialManager::get_filename(const Material* m) const {
	for (const auto&& [f, _m]: materials)
		if (_m == m)
			return f;
	return "";
}

string MaterialManager::describe(const Material* m, bool with_save_state) const {
	if (!m)
		return "[none]";
	string name = str(get_filename(m));
	if (with_save_state and has_changes(m))
		name += " *";
	if (name == "")
		name = "[internal]";
	if (m->parent)
		name += " < " + str(get_filename(m->parent));
	return name;
}

void MaterialManager::_load_from_file(Material* m, const Path &filename) {

	Configuration c;
	if (!c.load(filename)) {
		//if (engine.ignore_missing_files) {
			msg_error("material file missing: " + filename.str());
			return;
		/*} else {
			throw Exception("material file missing: " + filename.str());
		}*/
	}

	if (c.has("parent")) {
		auto parent = load(str(c.get("parent")));
		m->derive_from(parent);
	}

	if (c.has("color.albedo"))
		m->albedo = any2color(c.get("color.albedo"));
	if (c.has("color.roughness"))
		m->roughness = c.get_float("color.roughness", 0.5f);
	if (c.has("color.metal"))
		m->metal = c.get_float("color.metal", 0.1f);
	if (c.has("color.emission"))
		m->emission = any2color(c.get("color.emission"));

	if (c.has("textures")) {
		auto texture_files = c.get_str_array("textures");
		m->textures.resize(max(m->textures.num, texture_files.num));
		for (const auto& [i, f]: enumerate(texture_files))
			m->textures[i] = texture_manager->load_texture(f);
	}
	if (c.has("shader"))
		m->pass0.shader_path = c.get_str("shader", "");
	if (c.has("shadow.cast"))
		m->cast_shadow = c.get_bool("shadow.cast", true);

	if (c.has("friction.static"))
		m->friction._static = c.get_float("friction.static", 0.5f);
	if (c.has("friction.slide"))
		m->friction.sliding = c.get_float("friction.slide", 0.5f);
	if (c.has("friction.roll"))
		m->friction.rolling = c.get_float("friction.roll", 0.5f);
	if (c.has("friction.jump"))
		m->friction.jump = c.get_float("friction.jump", 0.5f);

	auto add_pass = [m] (int index) -> Material::RenderPassData& {
		m->set_num_passes(max(m->num_passes, index + 1));
		return m->pass(index);
	};
	auto try_parse_pass = [m, &c, add_pass] (const string& key, int index) {
		if (!c.has(key + ".mode"))
			return;
		auto& p = add_pass(index);
		p.shader_path = c.get_str(key + ".shader", "");
		string mode = c.get_str(key + ".cull", "ccw");
		p.cull_mode = CullMode::BACK;
		if (mode == "none")
			p.cull_mode = CullMode::NONE;
		else if (mode == "cw" or mode == "front")
			p.cull_mode = CullMode::FRONT;

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
		auto texture_files = c.get_str_array("reflection.cubemap");
		shared_array<Texture> cmt;
		for (auto &f: texture_files)
			cmt.add(texture_manager->load_texture(f));
		m->reflection.density = c.get_float("reflection.density", 1);
#if 0
			m->reflection.cube_map = new CubeMap(m->reflection.cube_map_size);
			for (int i=0;i<6;i++)
				m->reflection.cube_map->fill_side(i, cmt[i]);
#endif
	} else if (mode == "dynamic") {
		m->reflection.mode = ReflectionMode::CUBE_MAP_DYNAMIC;
		m->reflection.density = c.get_float("reflection.density", 1);
		m->reflection.cube_map_size = c.get_int("reflection.size", 128);
		//m->cube_map = FxCubeMapNew(m->cube_map_size);
		//FxCubeMapCreate(m->cube_map,cmt[0],cmt[1],cmt[2],cmt[3],cmt[4],cmt[5]);
	} else if (mode == "mirror") {
		m->reflection.mode = ReflectionMode::MIRROR;
	} else if (mode != "") {
		msg_error("unknown reflection mode: " + mode);
	}
}

string alpha_to_str(ygfx::Alpha a) {
	if (a == ygfx::Alpha::ZERO)
		return "zero";
	if (a == ygfx::Alpha::ONE)
		return "one";
	if (a == ygfx::Alpha::SOURCE_COLOR)
		return "source-color";
	if (a == ygfx::Alpha::SOURCE_INV_COLOR)
		return "source-inv-color";
	if (a == ygfx::Alpha::SOURCE_ALPHA)
		return "source-alpha";
	if (a == ygfx::Alpha::SOURCE_INV_ALPHA)
		return "source-inv-alpha";
	if (a == ygfx::Alpha::DEST_COLOR)
		return "dest-color";
	if (a == ygfx::Alpha::DEST_INV_COLOR)
		return "dest-inv-color";
	if (a == ygfx::Alpha::DEST_ALPHA)
		return "dest-alpha";
	if (a == ygfx::Alpha::DEST_INV_ALPHA)
		return "dest-inv-alpha";
	return "???";
}

Array<string> paths_to_str_arr(const Array<Path> &files) {
	Array<string> r;
	for (auto f: files)
		r.add(f.str());
	return r;
}

Any color2any(const color &c) {
	Any r = Any::EmptyList;
	r.add(c.r);
	r.add(c.g);
	r.add(c.b);
	r.add(c.a);
	return r;
}

void MaterialManager::_write_to_file(Material* material, const Path &filename) {

	Configuration c;

	Array<Path> texture_files;
	for (auto t: weak(material->textures))
		texture_files.add(texture_manager->get_filename(t));

	c.set_str_array("textures", paths_to_str_arr(texture_files));
	if (!material->cast_shadow)
		c.set_bool("cast-shadows", material->cast_shadow);

	c.set("color.albedo", color2any(material->albedo));
	if (material->emission != Black)
		c.set("color.emission", color2any(material->emission));
	c.set_float("color.roughness", material->roughness);
	c.set_float("color.metal", material->metal);

	for (int i=0; i<material->num_passes; i++) {
		auto &p = material->pass(i);
		string key = format("pass%d", i);
		c.set_str(key + ".shader", str(p.shader_path));

		if (p.mode == yrenderer::TransparencyMode::FACTOR) {
			c.set_str(key + ".mode", "factor");
			c.set_float(key + ".factor", p.factor);
		} else if (p.mode == yrenderer::TransparencyMode::FUNCTIONS) {
			c.set_str(key + ".mode", "function");
			c.set_str(key + ".source", alpha_to_str(p.source));
			c.set_str(key + ".dest", alpha_to_str(p.destination));
		} else if (p.mode == yrenderer::TransparencyMode::MIX) {
			c.set_str(key + ".mode", "mix");
		} else if (p.mode == yrenderer::TransparencyMode::COLOR_KEY_HARD) {
			c.set_str(key + ".mode", "key-hard");
		} else if (p.mode == yrenderer::TransparencyMode::COLOR_KEY_SMOOTH) {
			c.set_str(key + ".mode", "key-smooth");
		} else {
			c.set_str(key + ".mode", "solid");
		}
		if (p.cull_mode == ygfx::CullMode::NONE)
			c.set_str(key + ".cull", "none");
		else if (p.cull_mode == ygfx::CullMode::FRONT)
			c.set_str(key + ".cull", "front");
		if (p.mode != yrenderer::TransparencyMode::NONE or !p.z_buffer or !p.z_test)
			c.set_bool(key + ".z-write", p.z_buffer);
		if (!p.z_test)
			c.set_bool(key + ".z-test", p.z_test);
		/*if (material->transparency_mode != TransparencyMode::NONE) {
			c.set_bool("transparency.zbuffer", material->alpha_z_buffer);
		}*/
	}

	c.set_float("friction.static", material->friction._static);
	c.set_float("friction.slide", material->friction.sliding);
	c.set_float("friction.roll", material->friction.rolling);
	c.set_float("friction.jump", material->friction.jump);

	c.save(filename);
}

Material* MaterialManager::load(const Path& filename) {
	// an empty name loads the default material
	if (filename.is_empty())
		return default_material;

	for (auto&& [f, m]: materials)
		if (f == filename)
			return m;


	msg_write("loading material " + str(filename));

	auto m = new Material();
	materials.set(filename, m);
	_load_from_file(m, material_dir | filename.with(".material"));
	return m;
}

xfer<Material> MaterialManager::load_copy(const Path& filename) {
	return load(filename)->copy();
}

void MaterialManager::invalidate(Material* m) {
	having_changes.add(m);
	out_material_edited(m);
}

bool MaterialManager::has_changes(const Material *m) const {
	return having_changes.contains(m);
}

void MaterialManager::set_save_state(Material* m) {
	having_changes.erase(m);
}

Material* MaterialManager::create_internal() {
	auto m = new Material();
	m->textures = {texture_manager->load_texture("")};
	internal_materials.add(m);
	return m;
}

bool MaterialManager::is_from_file(const Material* m) const {
	for (const auto&& [f, _m]: materials)
		if (_m == m)
			return true;
	return false;
}
}

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

namespace yrenderer {

	using namespace ygfx;

MaterialManager::MaterialManager(Context* _ctx, const Path& _material_dir) {
	ctx = _ctx;
	shader_manager = ctx->shader_manager;
	material_dir = _material_dir;
	// create the default material
	trivial_material = new Material(ctx);
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
	if (!c.load(material_dir | filename.with(".material"))) {
		//if (engine.ignore_missing_files) {
			msg_error("material file missing: " + filename.str());
			return default_material->copy();
		/*} else {
			throw Exception("material file missing: " + filename.str());
		}*/
	}
	Material *m = new Material(ctx);

	m->albedo = any2color(c.get("color.albedo"));
	m->roughness = c.get_float("color.roughness", 0.5f);
	m->metal = c.get_float("color.metal", 0.1f);
	m->emission = any2color(c.get("color.emission"));

	auto texture_files = c.get_str_array("textures");
	for (auto &f: texture_files)
		m->textures.add(ctx->texture_manager->load_texture(f));
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
		texture_files = c.get_str_array("reflection.cubemap");
		shared_array<Texture> cmt;
		for (auto &f: texture_files)
			cmt.add(ctx->texture_manager->load_texture(f));
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

}

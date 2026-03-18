/*
 * FormatMaterial.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatMaterial.h"
#include "../../Session.h"
//#include "../../data/material/ShaderGraph.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/config.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../lib/os/msg.h"
#include "helper/ResourceManager.h"
#include "lib/yrenderer/MaterialManager.h"
#include "lib/yrenderer/TextureManager.h"

namespace yrenderer {
	color any2color(const Any &a);
}

namespace yrenderer {
	ygfx::Alpha parse_alpha(const string& s);
}

FormatMaterial::FormatMaterial(Session *s) : TypedFormat<DataMaterial>(s, FD_MATERIAL, "material", "Material", Flag::CANONICAL_READ_WRITE) {
}

Array<Path> str_arr_to_paths(const Array<string> &s) {
	Array<Path> r;
	for (auto &x: s)
		r.add(x);
	return r;
}

void FormatMaterial::load_current(const Path &filename, DataMaterial *data) {

	session->resource_manager->material_manager->_load_from_file(&data->material, filename);
#if 0
	Configuration c;
	c.load(filename);
	data->appearance.albedo = yrenderer::any2color(c.get("color.albedo", Any()));
	data->appearance.roughness = c.get_float("color.roughness", 0.5f);
	data->appearance.metal = c.get_float("color.metal", 0.1f);
	data->appearance.emissive = yrenderer::any2color(c.get("color.emission", Any()));
	data->appearance.cast_shadow = c.get_bool("cast-shadows", true);

	if (c.has("color.ambient")) {
		data->appearance.roughness = c.get_float("color.ambient", 0.5f);
		data->appearance.metal = c.get_float("color.specular", 0.5f);
	} else if (c.has("color.reflectivity")) {
		data->appearance.metal = c.get_float("color.reflectivity", 0.1f);
	}

	data->appearance.texture_files = str_arr_to_paths(c.get_str_array("textures"));

	auto try_read_pass = [this, &c, data] (int index, const string &key) {
		if (!c.has(key + ".mode"))
			return;
		if (index >= data->appearance.passes.num)
			data->appearance.passes.resize(index + 1);
		auto &p = data->appearance.passes[index];
		p.source = ygfx::Alpha::ONE;
		p.destination = ygfx::Alpha::ZERO;
#if ksdjfhskdjfh
		if (!p.shader.graph)
			p.shader.graph = new ShaderGraph(session);
#endif
		string m = c.get_str(key + ".mode", "none");
		if (m == "solid" or m == "none") {
			p.mode = yrenderer::TransparencyMode::NONE;
		} else if (m == "factor") {
			p.mode = yrenderer::TransparencyMode::FACTOR;
			p.factor = c.get_float(key + ".factor");
			p.z_write = false;
			p.z_test = true;
		} else if (m == "function") {
			p.mode = yrenderer::TransparencyMode::FUNCTIONS;
			p.source = yrenderer::parse_alpha(c.get_str(key + ".source", 0));
			p.destination = yrenderer::parse_alpha(c.get_str(key + ".dest", 0));
			p.z_write = false;
			p.z_test = true;
		} else if (m == "key-hard") {
			p.mode = yrenderer::TransparencyMode::COLOR_KEY_HARD;
		} else if (m == "key-smooth") {
			p.mode = yrenderer::TransparencyMode::COLOR_KEY_SMOOTH;
		} else if (m == "mix") {
			p.mode = yrenderer::TransparencyMode::MIX;
		} else if (m != "") {
			msg_error("unknown transparency mode: " + m);
		}
		p.shader.file = c.get_str(key + ".shader", "");
		m = c.get_str(key + ".cull", "");
		if (m == "none")
			p.culling = ygfx::CullMode::NONE;
		else if (m == "front" or m == "cw")
			p.culling = ygfx::CullMode::FRONT;
		m = c.get_str(key + ".z-write", "");
		if (m != "")
			p.z_write = m._bool();
		m = c.get_str(key + ".z-test", "");
		if (m != "")
			p.z_test = m._bool();
	};

	// deprecated
	if (c.has("shader"))
		data->appearance.passes[0].shader.file = c.get_str("shader", "");
	try_read_pass(0, "transparency");

	for (int i=0; i<8; i++)
		try_read_pass(i, format("pass%d", i));

	data->physics.friction_static = c.get_float("friction.static", 0.5f);
	data->physics.friction_sliding = c.get_float("friction.slide", 0.5f);
	data->physics.friction_rolling = c.get_float("friction.roll", 0.5f);
	data->physics.friction_jump = c.get_float("friction.jump", 0.5f);
#endif
}

void FormatMaterial::load_legacy(LegacyFile &lf, DataMaterial *data) {
	auto f = lf.f;
	if ((lf.ffv == 3) or (lf.ffv == 4)) {
		if (lf.ffv >= 4) {
			f->read_comment();
			int n = f->read_int();
			data->material.textures.resize(n);
			for (int i=0;i<n;i++)
				data->material.textures[i] = session->resource_manager->load_texture(f->read_str());
		}
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		read_color_argb(f, data->material.albedo);
		data->material.roughness = 0.5f;
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		/*shininess = */ f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		data->material.pass0.z_buffer = f->read_bool();
		data->material.pass0.z_test = true;
		// Appearance
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_bool();
		// Reflection
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_int();
		for (int i=0;i<6;i++)
			f->read_str();
		// ShaderFile
		f->read_comment();
		data->material.pass0.shader_path = f->read_str();
		// Physics
		f->read_comment();
		data->material.friction.jump = (float)f->read_int() * 0.001f;
		data->material.friction._static = (float)f->read_int() * 0.001f;
		data->material.friction.sliding = (float)f->read_int() * 0.001f;
		data->material.friction.rolling = (float)f->read_int() * 0.001f;
		/*data->material.vmin_jump =*/ (float)f->read_int() * 0.001f;
		/*data->material.vmin_sliding =*/ (float)f->read_int() * 0.001f;

		//AlphaZBuffer=(TransparencyMode!=TransparencyMode::FUNCTIONS)and(TransparencyMode!=TransparencyMode::FACTOR);
	}else if (lf.ffv==2){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->material.roughness = 0.5f;
		read_color_argb(f, data->material.albedo);
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		[[maybe_unused]] auto shininess = (float)f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->material.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			data->material.pass0.shader_path = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		data->material.friction.jump = (float)f->read_int() * 0.001f;
		data->material.friction._static = (float)f->read_int() * 0.001f;
		data->material.friction.sliding = (float)f->read_int() * 0.001f;
		data->material.friction.rolling = (float)f->read_int() * 0.001f;
		/*data->material.vmin_jump =*/ (float)f->read_int() * 0.001f;
		/*data->material.vmin_sliding =*/ (float)f->read_int() * 0.001f;

		data->material.pass0.z_buffer = (data->material.pass0.mode != yrenderer::TransparencyMode::FUNCTIONS) and (data->material.pass0.mode != yrenderer::TransparencyMode::FACTOR);
		data->material.pass0.z_test = true;
	}else if (lf.ffv==1){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->material.roughness = 0.5f;
		read_color_argb(f, data->material.albedo);
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		[[maybe_unused]] auto shininess = (float)f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->material.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		Path sf = f->read_str();
		if (!sf.is_empty())
		data->material.pass0.shader_path = sf.with(".fx.glsl");

		data->material.pass0.z_buffer = (data->material.pass0.mode != yrenderer::TransparencyMode::FUNCTIONS) and (data->material.pass0.mode != yrenderer::TransparencyMode::FACTOR);
		data->material.pass0.z_test = true;
	}else{
		//throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename, ffv, 1, 4));
	}
}

void FormatMaterial::_load(const Path &filename, DataMaterial *data, bool deep) {

	int ffv;
	data->reset();

	Stream *f = nullptr;


	auto lf = file_get_legacy_header(filename);
	//data->file_time = f->mtime().time;

	if (lf)
		load_legacy(*lf, data);
	else
		load_current(filename, data);

	/*if (ffv<0){
		throw FormatError(_("File format unreadable!"));*/

#if 0
	if (deep) {
		for (auto &p: data->material.passes)
			p.shader.load_from_file(data->doc);
	}
#endif

	delete f;
}




void FormatMaterial::_save(const Path &filename, DataMaterial *data) {
	session->resource_manager->material_manager->_write_to_file(&data->material, filename);
}

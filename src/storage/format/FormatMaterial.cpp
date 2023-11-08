/*
 * FormatMaterial.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatMaterial.h"
#include "../../Session.h"
#include "../../data/material/ShaderGraph.h"
#include "../../lib/hui/language.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/config.h"
#include "../../lib/os/formatter.h"
#include "../../lib/os/msg.h"

color any2color(const Any &a);
Any color2any(const color &c) {
	Any r = Any::EmptyArray;
	r.add(c.r);
	r.add(c.g);
	r.add(c.b);
	r.add(c.a);
	return r;
}

FormatMaterial::FormatMaterial(Session *s) : TypedFormat<DataMaterial>(s, FD_MATERIAL, "material", _("Material"), Flag::CANONICAL_READ_WRITE) {
}

Array<string> paths_to_str_arr(const Array<Path> &files) {
	Array<string> r;
	for (auto f: files)
		r.add(f.str());
	return r;
}

Array<Path> str_arr_to_paths(const Array<string> &s) {
	Array<Path> r;
	for (auto &x: s)
		r.add(x);
	return r;
}

void FormatMaterial::_load(const Path &filename, DataMaterial *data, bool deep) {

	int ffv;
	data->reset();

	TextLinesFormatter *f = nullptr;

	/*File *f = FileOpenText(filename);
	data->file_time = f->mtime().time;

	try {
		ffv=f->ReadFileFormatVersion();
	} catch (...) {*/
		ffv = -1;
		Configuration c;
		c.load(filename);
		data->appearance.albedo = any2color(c.get("color.albedo", Any()));
		data->appearance.roughness = c.get_float("color.roughness", 0.5f);
		data->appearance.metal = c.get_float("color.metal", 0.1f);
		data->appearance.emissive = any2color(c.get("color.emission", Any()));

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
			string m = c.get_str(key + ".mode", "");
			if (m == "factor") {
				p.mode = TransparencyMode::FACTOR;
				p.factor = c.get_float("transparency.factor");
				p.z_buffer = false;
			} else if (m == "function") {
				p.mode = TransparencyMode::FUNCTIONS;
				p.source = (nix::Alpha)c.get_int("transparency.source", 0);
				p.destination = (nix::Alpha)c.get_int("transparency.dest", 0);
				p.z_buffer = false;
			} else if (m == "key-hard") {
				p.mode = TransparencyMode::COLOR_KEY_HARD;
			} else if (m == "key-smooth") {
				p.mode = TransparencyMode::COLOR_KEY_SMOOTH;
			} else if (m == "mix") {
				p.mode = TransparencyMode::MIX;
			} else if (m != "") {
				msg_error("unknown transparency mode: " + m);
			}
			p.shader.file = c.get_str(key + ".shader", "");
			m = c.get_str(key + ".culling", "");
			if (m == "none")
				p.culling = 0;
			else if (m == "front" or m == "cw")
				p.culling = 2;
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
	//}

	/*if (ffv<0){
		throw FormatError(_("File format unreadable!"));
	}else*/ if ((ffv == 3) or (ffv == 4)){
		if (ffv >= 4){
			f->read_comment();
			int n = f->read_int();
			for (int i=0;i<n;i++)
				data->appearance.texture_files.add(f->read_str());
			if ((data->appearance.texture_files.num == 1) and (data->appearance.texture_files[0] == "")){
				data->appearance.texture_files.clear();
			}
		}
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		read_color_argb(f, data->appearance.albedo);
		data->appearance.roughness = 0.5f;
		read_color_argb(f, cc);
		data->appearance.metal = 0.1f;
		/*shininess = */ f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.passes[0].mode = (TransparencyMode)f->read_int();
		data->appearance.passes[0].factor = (float)f->read_int() * 0.01f;
		data->appearance.passes[0].source = (nix::Alpha)f->read_int();
		data->appearance.passes[0].destination = (nix::Alpha)f->read_int();
		data->appearance.passes[0].z_buffer = f->read_bool();
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
		data->appearance.passes[0].shader.file = f->read_str();
		// Physics
		f->read_comment();
		data->physics.friction_jump = (float)f->read_int() * 0.001f;
		data->physics.friction_static = (float)f->read_int() * 0.001f;
		data->physics.friction_sliding = (float)f->read_int() * 0.001f;
		data->physics.friction_rolling = (float)f->read_int() * 0.001f;
		data->physics.vmin_jump = (float)f->read_int() * 0.001f;
		data->physics.vmin_sliding = (float)f->read_int() * 0.001f;
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->read_intC();
			data->Sound.NumRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyMode::FUNCTIONS)and(TransparencyMode!=TransparencyMode::FACTOR);
	}else if (ffv==2){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->appearance.roughness = 0.5f;
		read_color_argb(f, data->appearance.albedo);
		read_color_argb(f, cc);
		data->appearance.metal = 0.1f;
		/*shininess =*/ (float)f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.passes[0].mode = (TransparencyMode)f->read_int();
		data->appearance.passes[0].factor = (float)f->read_int() * 0.01f;
		data->appearance.passes[0].source = (nix::Alpha)f->read_int();
		data->appearance.passes[0].destination = (nix::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->appearance.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			data->appearance.passes[0].shader.file = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		data->physics.friction_jump = (float)f->read_int() * 0.001f;
		data->physics.friction_static = (float)f->read_int() * 0.001f;
		data->physics.friction_sliding = (float)f->read_int() * 0.001f;
		data->physics.friction_rolling = (float)f->read_int() * 0.001f;
		data->physics.vmin_jump = (float)f->read_int() * 0.001f;
		data->physics.vmin_sliding = (float)f->read_int() * 0.001f;

		data->appearance.passes[0].z_buffer=(data->appearance.passes[0].mode != TransparencyMode::FUNCTIONS) and (data->appearance.passes[0].mode != TransparencyMode::FACTOR);
	}else if (ffv==1){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->appearance.roughness = 0.5f;
		read_color_argb(f, data->appearance.albedo);
		read_color_argb(f, cc);
		data->appearance.metal = 0.1f;
		/*shininess =*/ (float)f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.passes[0].mode = (TransparencyMode)f->read_int();
		data->appearance.passes[0].factor = (float)f->read_int() * 0.01f;
		data->appearance.passes[0].source = (nix::Alpha)f->read_int();
		data->appearance.passes[0].destination = (nix::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->appearance.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		Path sf = f->read_str();
		if (!sf.is_empty())
			data->appearance.passes[0].shader.file = sf.with(".fx.glsl");

		data->appearance.passes[0].z_buffer = (data->appearance.passes[0].mode != TransparencyMode::FUNCTIONS) and (data->appearance.passes[0].mode != TransparencyMode::FACTOR);
	}else{
		//throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename, ffv, 1, 4));
	}

	if (deep) {
		for (auto &p: data->appearance.passes)
			p.shader.load_from_file(session);
	}

	delete f;
}

void FormatMaterial::_save(const Path &filename, DataMaterial *data) {
	Configuration c;

	c.set_str_array("textures", paths_to_str_arr(data->appearance.texture_files));

	c.set("color.albedo", color2any(data->appearance.albedo));
	if (data->appearance.emissive != Black)
		c.set("color.emission", color2any(data->appearance.emissive));
	c.set_float("color.roughness", data->appearance.roughness);
	c.set_float("color.metal", data->appearance.metal);

	for (int i=0; i<data->appearance.passes.num; i++) {
		auto &p = data->appearance.passes[i];
		string key = format("pass%d", i);
		c.set_str(key + ".shader", p.shader.file.str());

		if (p.mode == TransparencyMode::FACTOR) {
			c.set_str(key + ".mode", "factor");
			c.set_float(key + ".factor", p.factor);
		} else if (p.mode == TransparencyMode::FUNCTIONS) {
			c.set_str(key + ".mode", "function");
			c.set_int(key + ".source", (int)p.source);
			c.set_int(key + ".dest", (int)p.destination);
		} else if (p.mode == TransparencyMode::MIX) {
			c.set_str(key + ".mode", "mix");
		} else if (p.mode == TransparencyMode::COLOR_KEY_HARD) {
			c.set_str(key + ".mode", "key-hard");
		} else if (p.mode == TransparencyMode::COLOR_KEY_SMOOTH) {
			c.set_str(key + ".mode", "key-smooth");
		} else {
			c.set_str(key + ".mode", "solid");
		}
		if (p.culling == 0)
			c.set_str(key + ".culling", "none");
		else if (p.culling == 2)
			c.set_str(key + ".culling", "front");
		/*if (data->appearance.transparency_mode != TransparencyMode::NONE) {
			c.set_bool("transparency.zbuffer", data->appearance.alpha_z_buffer);
		}*/
	}

	c.set_float("friction.static", data->physics.friction_static);
	c.set_float("friction.slide", data->physics.friction_sliding);
	c.set_float("friction.roll", data->physics.friction_rolling);
	c.set_float("friction.jump", data->physics.friction_jump);

	c.save(filename);
}

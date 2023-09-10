/*
 * FormatMaterial.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatMaterial.h"
#include "../../EdwardWindow.h"
#include "../../data/material/ShaderGraph.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/config.h"
#include "../../lib/os/formatter.h"

color any2color(const Any &a);
Any color2any(const color &c) {
	Any r = Any::EmptyArray;
	r.add(c.r);
	r.add(c.g);
	r.add(c.b);
	r.add(c.a);
	return r;
}

FormatMaterial::FormatMaterial(EdwardWindow *ed) : TypedFormat<DataMaterial>(ed, FD_MATERIAL, "material", _("Material"), Flag::CANONICAL_READ_WRITE) {
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
		data->shader.file = c.get_str("shader", "");

		data->physics.friction_static = c.get_float("friction.static", 0.5f);
		data->physics.friction_sliding = c.get_float("friction.slide", 0.5f);
		data->physics.friction_rolling = c.get_float("friction.roll", 0.5f);
		data->physics.friction_jump = c.get_float("friction.jump", 0.5f);

		string m = c.get_str("transparency.mode", "");
		if (m == "factor") {
			data->appearance.transparency_mode = TransparencyMode::FACTOR;
			data->appearance.alpha_factor = c.get_float("transparency.factor");
			data->appearance.alpha_z_buffer = false;
		} else if (m == "function") {
			data->appearance.transparency_mode = TransparencyMode::FUNCTIONS;
			data->appearance.alpha_source = (nix::Alpha)c.get_int("transparency.source", 0);
			data->appearance.alpha_destination = (nix::Alpha)c.get_int("transparency.dest", 0);
			data->appearance.alpha_z_buffer = false;
		} else if (m == "key-hard") {
			data->appearance.transparency_mode = TransparencyMode::COLOR_KEY_HARD;
		} else if (m == "key-smooth") {
			data->appearance.transparency_mode = TransparencyMode::COLOR_KEY_SMOOTH;
		} else if (m != "") {
			msg_error("unknown transparency mode: " + m);
		}

		m = c.get_str("reflection.mode", "");

		if (m == "static") {
			data->appearance.reflection_mode = ReflectionMode::CUBE_MAP_STATIC;
			data->appearance.reflection_texture_file = str_arr_to_paths(c.get_str_array("reflection.cubemap"));
			data->appearance.reflection_texture_file.resize(6);
			data->appearance.reflection_density = c.get_float("reflection.density", 1);
		} else if (m == "dynamic") {
			data->appearance.reflection_mode = ReflectionMode::CUBE_MAP_DYNAMIC;
			data->appearance.reflection_density = c.get_float("reflection.density", 1);
			data->appearance.reflection_size = c.get_float("reflection.size", 128);
		} else if (m == "mirror") {
			data->appearance.reflection_mode = ReflectionMode::MIRROR;
		} else if (m != "") {
			msg_error("unknown reflection mode: " + m);
		}
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
		data->appearance.transparency_mode = (TransparencyMode)f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = (nix::Alpha)f->read_int();
		data->appearance.alpha_destination = (nix::Alpha)f->read_int();
		data->appearance.alpha_z_buffer = f->read_bool();
		// Appearance
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_bool();
		// Reflection
		f->read_comment();
		data->appearance.reflection_mode = (ReflectionMode)f->read_int();
		data->appearance.reflection_density = (float)f->read_int();
		data->appearance.reflection_size = f->read_int();
		for (int i=0;i<6;i++)
			data->appearance.reflection_texture_file[i] = f->read_str();
		// ShaderFile
		f->read_comment();
		data->shader.file = f->read_str();
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
		data->appearance.transparency_mode = (TransparencyMode)f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = (nix::Alpha)f->read_int();
		data->appearance.alpha_destination = (nix::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			data->appearance.reflection_mode = ReflectionMode::METAL;
			data->appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		if (Mirror)
			data->appearance.reflection_mode = ReflectionMode::MIRROR;
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			data->shader.file = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		data->physics.friction_jump = (float)f->read_int() * 0.001f;
		data->physics.friction_static = (float)f->read_int() * 0.001f;
		data->physics.friction_sliding = (float)f->read_int() * 0.001f;
		data->physics.friction_rolling = (float)f->read_int() * 0.001f;
		data->physics.vmin_jump = (float)f->read_int() * 0.001f;
		data->physics.vmin_sliding = (float)f->read_int() * 0.001f;

		data->appearance.alpha_z_buffer=(data->appearance.transparency_mode != TransparencyMode::FUNCTIONS) and (data->appearance.transparency_mode != TransparencyMode::FACTOR);
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
		data->appearance.transparency_mode = (TransparencyMode)f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = (nix::Alpha)f->read_int();
		data->appearance.alpha_destination = (nix::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			data->appearance.reflection_mode = ReflectionMode::METAL;
			data->appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		data->appearance.reflection_mode = (f->read_bool() ? ReflectionMode::MIRROR : ReflectionMode::NONE);
		bool Mirror = f->read_bool();
		if (Mirror)
			data->appearance.reflection_mode = ReflectionMode::MIRROR;
		// ShaderFile
		f->read_comment();
		Path sf = f->read_str();
		if (!sf.is_empty())
			data->shader.file = sf.with(".fx.glsl");

		data->appearance.alpha_z_buffer = (data->appearance.transparency_mode != TransparencyMode::FUNCTIONS) and (data->appearance.transparency_mode != TransparencyMode::FACTOR);
	}else{
		//throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename, ffv, 1, 4));
	}

	if (deep) {
		data->shader.load_from_file(ed);
	}

	delete(f);
}

void FormatMaterial::_save(const Path &filename, DataMaterial *data) {
	Configuration c;

	c.set_str_array("textures", paths_to_str_arr(data->appearance.texture_files));
	c.set_str("shader", data->shader.file.str());

	c.set("color.albedo", color2any(data->appearance.albedo));
	if (data->appearance.emissive != Black)
		c.set("color.emission", color2any(data->appearance.emissive));
	c.set_float("color.roughness", data->appearance.roughness);
	c.set_float("color.metal", data->appearance.metal);

	if (data->appearance.transparency_mode == TransparencyMode::FACTOR) {
		c.set_str("transparency.mode", "factor");
		c.set_float("transparency.factor", data->appearance.alpha_factor);
	} else if (data->appearance.transparency_mode == TransparencyMode::FUNCTIONS) {
		c.set_str("transparency.mode", "function");
		c.set_int("transparency.source", (int)data->appearance.alpha_source);
		c.set_int("transparency.dest", (int)data->appearance.alpha_destination);
	} else if (data->appearance.transparency_mode == TransparencyMode::COLOR_KEY_HARD) {
		c.set_str("transparency.mode", "key-hard");
	} else if (data->appearance.transparency_mode == TransparencyMode::COLOR_KEY_SMOOTH) {
		c.set_str("transparency.mode", "key-smooth");
	}
	/*if (data->appearance.transparency_mode != TransparencyMode::NONE) {
		c.set_bool("transparency.zbuffer", data->appearance.alpha_z_buffer);
	}*/

	if (data->appearance.reflection_mode == ReflectionMode::CUBE_MAP_STATIC) {
		c.set_str("reflection.mode", "static");
		c.set_str_array("reflection.cubemap", paths_to_str_arr(data->appearance.reflection_texture_file));
		c.set_float("reflection.density", data->appearance.reflection_density);
	} else if (data->appearance.reflection_mode == ReflectionMode::CUBE_MAP_DYNAMIC) {
		c.set_str("reflection.mode", "dynamic");
		c.set_float("reflection.density", data->appearance.reflection_density);
		c.set_int("reflection.size", data->appearance.reflection_size);
	} else if (data->appearance.reflection_mode == ReflectionMode::MIRROR) {
		c.set_str("reflection.mode", "mirror");
	}

	c.set_float("friction.static", data->physics.friction_static);
	c.set_float("friction.slide", data->physics.friction_sliding);
	c.set_float("friction.roll", data->physics.friction_rolling);
	c.set_float("friction.jump", data->physics.friction_jump);

	c.save(filename);
}

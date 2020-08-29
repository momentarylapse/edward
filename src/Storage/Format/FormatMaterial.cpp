/*
 * FormatMaterial.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatMaterial.h"
#include "../../Edward.h"
#include "../../Data/Material/ShaderGraph.h"
#include "../../lib/nix/nix.h"

FormatMaterial::FormatMaterial() : TypedFormat<DataMaterial>(FD_MATERIAL, "material", _("Material"), Flag::CANONICAL_READ_WRITE) {
}

string paths_to_str(const Array<Path> &files) {
	Array<string> r;
	for (auto f: files)
		r.add(f.str());
	return implode(r, ",");
}

Array<Path> str_to_paths(const string &s) {
	Array<Path> r;
	for (auto &x: s.explode(","))
		r.add(x);
	return r;
}

void FormatMaterial::_load(const Path &filename, DataMaterial *data, bool deep) {

	int ffv;
	data->reset();

	File *f = FileOpenText(filename);
	data->file_time = f->mtime().time;

	try {
		ffv=f->ReadFileFormatVersion();
	} catch (...) {
		hui::Configuration c;
		c.load(filename);
		data->appearance.diffuse = color::parse(c.get_str("color.albedo", ""));
		data->appearance.ambient = c.get_float("color.ambient", 0.5f);
		data->appearance.specular = c.get_float("color.specular", 0.1f);
		data->appearance.shininess = c.get_float("color.shininess", 10);
		data->appearance.emissive = color::parse(c.get_str("color.emission", ""));

		data->appearance.texture_files = str_to_paths(c.get_str("textures", ""));
		data->appearance.shader_file = c.get_str("shader", "");

		data->physics.friction_static = c.get_float("friction.static", 0.5f);
		data->physics.friction_sliding = c.get_float("friction.slide", 0.5f);
		data->physics.friction_rolling = c.get_float("friction.roll", 0.5f);
		data->physics.friction_jump = c.get_float("friction.jump", 0.5f);

		string m = c.get_str("transparency.mode", "");
		if (m == "factor") {
			data->appearance.transparency_mode = TRANSPARENCY_FACTOR;
			data->appearance.alpha_factor = c.get_float("transparency.factor");
			data->appearance.alpha_z_buffer = false;
		} else if (m == "function") {
			data->appearance.transparency_mode = TRANSPARENCY_FUNCTIONS;
			data->appearance.alpha_source = c.get_int("transparency.source", 0);
			data->appearance.alpha_destination = c.get_int("transparency.dest", 0);
			data->appearance.alpha_z_buffer = false;
		} else if (m == "key-hard") {
			data->appearance.transparency_mode = TRANSPARENCY_COLOR_KEY_HARD;
		} else if (m == "key-smooth") {
			data->appearance.transparency_mode = TRANSPARENCY_COLOR_KEY_SMOOTH;
		} else if (m != "") {
			msg_error("unknown transparency mode: " + m);
		}

		m = c.get_str("reflection.mode", "");

		if (m == "static") {
			data->appearance.reflection_mode = REFLECTION_CUBE_MAP_STATIC;
			data->appearance.reflection_texture_file = str_to_paths(c.get_str("reflection.cubemap", ""));
			data->appearance.reflection_texture_file.resize(6);
			data->appearance.reflection_density = c.get_float("reflection.density", 1);
		} else if (m == "dynamic") {
			data->appearance.reflection_mode = REFLECTION_CUBE_MAP_DYNAMIC;
			data->appearance.reflection_density = c.get_float("reflection.density", 1);
			data->appearance.reflection_size = c.get_float("reflection.size", 128);
		} else if (m == "mirror") {
			data->appearance.reflection_mode = REFLECTION_MIRROR;
		} else if (m != "") {
			msg_error("unknown reflection mode: " + m);
		}
	}

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
		read_color_argb(f, data->appearance.diffuse);
		data->appearance.ambient = 0.5f;
		read_color_argb(f, cc);
		data->appearance.specular = 0.1f;
		data->appearance.shininess = f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.transparency_mode = f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = f->read_int();
		data->appearance.alpha_destination = f->read_int();
		data->appearance.alpha_z_buffer = f->read_bool();
		// Appearance
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_bool();
		// Reflection
		f->read_comment();
		data->appearance.reflection_mode = f->read_int();
		data->appearance.reflection_density = (float)f->read_int();
		data->appearance.reflection_size = f->read_int();
		for (int i=0;i<6;i++)
			data->appearance.reflection_texture_file[i] = f->read_str();
		// ShaderFile
		f->read_comment();
		data->appearance.shader_file = f->read_str();
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

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)and(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->appearance.ambient = 0.5f;
		read_color_argb(f, data->appearance.diffuse);
		read_color_argb(f, cc);
		data->appearance.specular = 0.1f;
		data->appearance.shininess = (float)f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.transparency_mode = f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = f->read_int();
		data->appearance.alpha_destination = f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			data->appearance.reflection_mode = REFLECTION_METAL;
			data->appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		if (Mirror)
			data->appearance.reflection_mode = REFLECTION_MIRROR;
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			data->appearance.shader_file = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		data->physics.friction_jump = (float)f->read_int() * 0.001f;
		data->physics.friction_static = (float)f->read_int() * 0.001f;
		data->physics.friction_sliding = (float)f->read_int() * 0.001f;
		data->physics.friction_rolling = (float)f->read_int() * 0.001f;
		data->physics.vmin_jump = (float)f->read_int() * 0.001f;
		data->physics.vmin_sliding = (float)f->read_int() * 0.001f;

		data->appearance.alpha_z_buffer=(data->appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (data->appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else if (ffv==1){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->appearance.ambient = 0.5f;
		read_color_argb(f, data->appearance.diffuse);
		read_color_argb(f, cc);
		data->appearance.specular = 0.1f;
		data->appearance.shininess = (float)f->read_int();
		read_color_argb(f, data->appearance.emissive);
		// Transparency
		f->read_comment();
		data->appearance.transparency_mode = f->read_int();
		data->appearance.alpha_factor = (float)f->read_int() * 0.01f;
		data->appearance.alpha_source = f->read_int();
		data->appearance.alpha_destination = f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			data->appearance.reflection_mode = REFLECTION_METAL;
			data->appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		data->appearance.reflection_mode = (f->read_bool() ? REFLECTION_MIRROR : REFLECTION_NONE);
		bool Mirror = f->read_bool();
		if (Mirror)
			data->appearance.reflection_mode = REFLECTION_MIRROR;
		// ShaderFile
		f->read_comment();
		Path sf = f->read_str();
		if (!sf.is_empty())
			data->appearance.shader_file = sf.with(".fx.glsl");

		data->appearance.alpha_z_buffer = (data->appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (data->appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else{
		//throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename, ffv, 1, 4));
	}

	if (deep) {
		data->appearance.update_shader_from_file();
	}

	delete(f);
}

void FormatMaterial::_save(const Path &filename, DataMaterial *data) {
	hui::Configuration c;

	c.set_str("textures", paths_to_str(data->appearance.texture_files));
	c.set_str("shader", data->appearance.shader_file.str());

	c.set_str("color.albedo", data->appearance.diffuse.str());
	if (data->appearance.emissive != Black)
		c.set_str("color.emission", data->appearance.emissive.str());
	c.set_float("color.ambient", data->appearance.ambient);
	c.set_float("color.specular", data->appearance.specular);
	c.set_float("color.shininess", data->appearance.shininess);

	if (data->appearance.transparency_mode == TRANSPARENCY_FACTOR) {
		c.set_str("transparency.mode", "factor");
		c.set_float("transparency.factor", data->appearance.alpha_factor);
	} else if (data->appearance.transparency_mode == TRANSPARENCY_FUNCTIONS) {
		c.set_str("transparency.mode", "function");
		c.set_int("transparency.source", data->appearance.alpha_source);
		c.set_int("transparency.dest", data->appearance.alpha_destination);
	} else if (data->appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_HARD) {
		c.set_str("transparency.mode", "key-hard");
	} else if (data->appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_SMOOTH) {
		c.set_str("transparency.mode", "key-smooth");
	}
	/*if (data->appearance.transparency_mode != TRANSPARENCY_NONE) {
		c.set_bool("transparency.zbuffer", data->appearance.alpha_z_buffer);
	}*/

	if (data->appearance.reflection_mode == REFLECTION_CUBE_MAP_STATIC) {
		c.set_str("reflection.mode", "static");
		c.set_str("reflection.cubemap", paths_to_str(data->appearance.reflection_texture_file));
		c.set_float("reflection.density", data->appearance.reflection_density);
	} else if (data->appearance.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC) {
		c.set_str("reflection.mode", "dynamic");
		c.set_float("reflection.density", data->appearance.reflection_density);
		c.set_int("reflection.size", data->appearance.reflection_size);
	} else if (data->appearance.reflection_mode == REFLECTION_MIRROR) {
		c.set_str("reflection.mode", "mirror");
	}

	c.set_float("friction.static", data->physics.friction_static);
	c.set_float("friction.slide", data->physics.friction_sliding);
	c.set_float("friction.roll", data->physics.friction_rolling);
	c.set_float("friction.jump", data->physics.friction_jump);

	c.save(filename);
}

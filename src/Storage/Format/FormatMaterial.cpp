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

void FormatMaterial::_load(const string &filename, DataMaterial *data, bool deep) {

	int ffv;
	data->reset();

	File *f = FileOpenText(filename);
	data->file_time = f->mtime().time;

	ffv=f->ReadFileFormatVersion();
	if (ffv<0){
		throw FormatError(_("File format unreadable!"));
	}else if ((ffv == 3) or (ffv == 4)){
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
		read_color_argb(f, data->appearance.ambient);
		read_color_argb(f, data->appearance.diffuse);
		read_color_argb(f, data->appearance.specular);
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
		read_color_argb(f, data->appearance.ambient);
		read_color_argb(f, data->appearance.diffuse);
		read_color_argb(f, data->appearance.specular);
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
		read_color_argb(f, data->appearance.ambient);
		read_color_argb(f, data->appearance.diffuse);
		read_color_argb(f, data->appearance.specular);
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
		string sf = f->read_str();
		if (sf.num > 0)
			data->appearance.shader_file = sf + ".fx.glsl";

		data->appearance.alpha_z_buffer = (data->appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (data->appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else{
		throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 1, 4));
	}

	if (deep) {
		data->appearance.update_shader_from_file();
	}

	delete(f);

}

void FormatMaterial::_save(const string &filename, DataMaterial *data) {

	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 4);

	f->write_comment("// Textures");
	f->write_int(data->appearance.texture_files.num);
	for (string &tf: data->appearance.texture_files)
		f->write_str(tf);
	f->write_comment("// Colors");
	write_color_argb(f, data->appearance.ambient);
	write_color_argb(f, data->appearance.diffuse);
	write_color_argb(f, data->appearance.specular);
	f->write_int(data->appearance.shininess);
	write_color_argb(f, data->appearance.emissive);
	f->write_comment("// Transparency");
	f->write_int(data->appearance.transparency_mode);
	f->write_int(data->appearance.alpha_factor * 100.0f);
	f->write_int(data->appearance.alpha_source);
	f->write_int(data->appearance.alpha_destination);
	f->write_bool(data->appearance.alpha_z_buffer);
	f->write_comment("// Appearance");
	f->write_int(0);
	f->write_int(0);
	f->write_bool(false);
	f->write_comment("// Reflection");
	f->write_int(data->appearance.reflection_mode);
	f->write_int(data->appearance.reflection_density);
	f->write_int(data->appearance.reflection_size);
	for (int i=0;i<6;i++)
		f->write_str(data->appearance.reflection_texture_file[i]);
	f->write_comment("// ShaderFile");
	f->write_str(data->appearance.shader_file);
	f->write_comment("// Physics");
	f->write_int(data->physics.friction_jump * 1000.0f);
	f->write_int(data->physics.friction_static * 1000.0f);
	f->write_int(data->physics.friction_sliding * 1000.0f);
	f->write_int(data->physics.friction_rolling * 1000.0f);
	f->write_int(data->physics.vmin_jump * 1000.0f);
	f->write_int(data->physics.vmin_sliding * 1000.0f);
	f->write_comment("// Sound");
	f->write_int(data->Sound.NumRules);
	for (int i=0;i<data->Sound.NumRules;i++){
	}


	f->write_str("#");
	delete(f);
}

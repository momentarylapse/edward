/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "../../Edward.h"
#include "../../lib/nix/nix.h"



DataMaterial::DataMaterial() :
	Data(FD_MATERIAL)
{
	appearance.shader = NULL;
	appearance.cube_map = new nix::CubeMap(128);

	reset();
}

DataMaterial::~DataMaterial()
{
	reset();
}

bool DataMaterial::save(const string & _filename)
{
	filename = _filename;
	ed->make_dirs(filename);

	File *f = FileCreateText(filename);
	f->WriteFileFormatVersion(false, 4);

	f->write_comment("// Textures");
	f->write_int(appearance.texture_files.num);
	for (string &tf: appearance.texture_files)
		f->write_str(tf);
	f->write_comment("// Colors");
	write_color_argb(f, appearance.ambient);
	write_color_argb(f, appearance.diffuse);
	write_color_argb(f, appearance.specular);
	f->write_int(appearance.shininess);
	write_color_argb(f, appearance.emissive);
	f->write_comment("// Transparency");
	f->write_int(appearance.transparency_mode);
	f->write_int(appearance.alpha_factor * 100.0f);
	f->write_int(appearance.alpha_source);
	f->write_int(appearance.alpha_destination);
	f->write_bool(appearance.alpha_z_buffer);
	f->write_comment("// Appearance");
	f->write_int(0);
	f->write_int(0);
	f->write_bool(false);
	f->write_comment("// Reflection");
	f->write_int(appearance.reflection_mode);
	f->write_int(appearance.reflection_density);
	f->write_int(appearance.reflection_size);
	for (int i=0;i<6;i++)
		f->write_str(appearance.reflection_texture_file[i]);
	f->write_comment("// ShaderFile");
	f->write_str(appearance.shader_file);
	f->write_comment("// Physics");
	f->write_int(physics.friction_jump * 1000.0f);
	f->write_int(physics.friction_static * 1000.0f);
	f->write_int(physics.friction_sliding * 1000.0f);
	f->write_int(physics.friction_rolling * 1000.0f);
	f->write_int(physics.vmin_jump * 1000.0f);
	f->write_int(physics.vmin_sliding * 1000.0f);
	f->write_comment("// Sound");
	f->write_int(Sound.NumRules);
	for (int i=0;i<Sound.NumRules;i++){
	}


	f->write_str("#");
	delete(f);

	action_manager->mark_current_as_save();
	return true;
}



bool DataMaterial::load(const string & _filename, bool deep)
{
	bool error=false;
	int ffv;
	reset();

	filename = _filename;
	ed->make_dirs(filename);
	File *f = FileOpenText(filename);
	if (!f){
		ed->set_message(_("Can't load material file"));
		return false;
	}
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();
	if (ffv<0){
		ed->error_box(_("File format unreadable!"));
		error=true;
	}else if ((ffv == 3) or (ffv == 4)){
		if (ffv >= 4){
			f->read_comment();
			int n = f->read_int();
			for (int i=0;i<n;i++)
				appearance.texture_files.add(f->read_str());
			if ((appearance.texture_files.num == 1) and (appearance.texture_files[0] == "")){
				appearance.texture_files.clear();
			}
		}
		// Colors
		f->read_comment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = f->read_int();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->read_comment();
		appearance.transparency_mode = f->read_int();
		appearance.alpha_factor = (float)f->read_int() * 0.01f;
		appearance.alpha_source = f->read_int();
		appearance.alpha_destination = f->read_int();
		appearance.alpha_z_buffer = f->read_bool();
		// Appearance
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_bool();
		// Reflection
		f->read_comment();
		appearance.reflection_mode = f->read_int();
		appearance.reflection_density = (float)f->read_int();
		appearance.reflection_size = f->read_int();
		for (int i=0;i<6;i++)
			appearance.reflection_texture_file[i] = f->read_str();
		// ShaderFile
		f->read_comment();
		appearance.shader_file = f->read_str();
		// Physics
		f->read_comment();
		physics.friction_jump = (float)f->read_int() * 0.001f;
		physics.friction_static = (float)f->read_int() * 0.001f;
		physics.friction_sliding = (float)f->read_int() * 0.001f;
		physics.friction_rolling = (float)f->read_int() * 0.001f;
		physics.vmin_jump = (float)f->read_int() * 0.001f;
		physics.vmin_sliding = (float)f->read_int() * 0.001f;
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->read_intC();
			Sound.NumRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)and(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->read_comment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = (float)f->read_int();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->read_comment();
		appearance.transparency_mode = f->read_int();
		appearance.alpha_factor = (float)f->read_int() * 0.01f;
		appearance.alpha_source = f->read_int();
		appearance.alpha_destination = f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			appearance.reflection_mode = REFLECTION_METAL;
			appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		if (Mirror)
			appearance.reflection_mode = REFLECTION_MIRROR;
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			appearance.shader_file = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		physics.friction_jump = (float)f->read_int() * 0.001f;
		physics.friction_static = (float)f->read_int() * 0.001f;
		physics.friction_sliding = (float)f->read_int() * 0.001f;
		physics.friction_rolling = (float)f->read_int() * 0.001f;
		physics.vmin_jump = (float)f->read_int() * 0.001f;
		physics.vmin_sliding = (float)f->read_int() * 0.001f;

		appearance.alpha_z_buffer=(appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else if (ffv==1){
		// Colors
		f->read_comment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = (float)f->read_int();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->read_comment();
		appearance.transparency_mode = f->read_int();
		appearance.alpha_factor = (float)f->read_int() * 0.01f;
		appearance.alpha_source = f->read_int();
		appearance.alpha_destination = f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0){
			appearance.reflection_mode = REFLECTION_METAL;
			appearance.reflection_density = (float)MetalDensity;
		}
		f->read_int();
		f->read_int();
		appearance.reflection_mode = (f->read_bool() ? REFLECTION_MIRROR : REFLECTION_NONE);
		bool Mirror = f->read_bool();
		if (Mirror)
			appearance.reflection_mode = REFLECTION_MIRROR;
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			appearance.shader_file = sf + ".fx.glsl";

		appearance.alpha_z_buffer = (appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else{
		ed->error_box(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 1, 4));
		error = true;
	}

	delete(f);


	if (deep)
		UpdateTextures();

	reset_history();
	notify();
	return !error;
}


void DataMaterial::AppearanceData::reset()
{
	texture_files.clear();
	textures.clear();

	ambient = White;
	diffuse = White;
	specular = Black;
	shininess = 20;
	emissive = Black;

	transparency_mode = TRANSPARENCY_NONE;
	alpha_source = alpha_destination = 0;
	alpha_factor = 0.5f;
	alpha_z_buffer = true;

	reflection_mode = REFLECTION_NONE;
	reflection_density = 0;
	reflection_size = 128;
	for (int i=0;i<6;i++)
		reflection_texture_file[i] = "";


	shader_file.clear();
}

nix::Shader *DataMaterial::AppearanceData::get_shader() const
{
	return nix::LoadShader(shader_file);
}


void DataMaterial::PhysicsData::Reset()
{
	friction_jump = 0.7f;
	friction_static = 0.8f;
	friction_sliding = 0.5f;
	friction_rolling = 0.2f;
	vmin_jump = 10;
	vmin_sliding = 10;
	Burnable = false;
	BurningTemperature = 500;
	BurningIntensity = 40;
}


void DataMaterial::SoundData::Reset()
{
	NumRules = 0;
}

void DataMaterial::reset()
{
	filename = "";

	if (appearance.shader)
		appearance.shader->unref();
	appearance.shader = NULL;

	appearance.reset();
	physics.Reset();
	Sound.Reset();


	reset_history();
	notify();
}

void DataMaterial::ApplyForRendering()
{
	nix::SetMaterial(appearance.ambient, appearance.diffuse, appearance.specular, appearance.shininess, appearance.emissive);

	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(true, true);
	if (appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_HARD){
		nix::SetAlpha(ALPHA_COLOR_KEY_HARD);
	}else if (appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_SMOOTH){
		nix::SetAlpha(ALPHA_COLOR_KEY_SMOOTH);
	}else if (appearance.transparency_mode == TRANSPARENCY_FUNCTIONS){
		nix::SetAlpha(appearance.alpha_source, appearance.alpha_destination);
		nix::SetZ(false, false);
	}else if (appearance.transparency_mode == TRANSPARENCY_FACTOR){
		nix::SetAlpha(appearance.alpha_factor);
		nix::SetZ(false, false);
	}

	nix::SetShader(appearance.shader);

	nix::SetTextures(appearance.textures);
}

void create_fake_dynamic_cube_map(nix::CubeMap *cube_map)
{
	Image im;
	int size = cube_map->width;
	im.create(size, size, White);
	for (int i=0; i<size; i++)
		for (int j=0; j<size; j++){
			float f = 0.2;
			if ((i % 16) == 0 or (j % 16) == 0)
				f = 0.5;
			if ((i % 64) == 0 or (j % 64) == 0)
				f = 1;
			im.set_pixel(i, j, color(1, f, f, f));
		}
	for (int i=0;i<6;i++)
		cube_map->overwrite_side(i, im);
}

void DataMaterial::UpdateTextures()
{
	appearance.textures.clear();
	for (string &tf: appearance.texture_files)
		appearance.textures.add(nix::LoadTexture(tf));
	if (appearance.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC){
		create_fake_dynamic_cube_map(appearance.cube_map);
		appearance.textures.add(appearance.cube_map);
	}else if (appearance.reflection_mode == REFLECTION_CUBE_MAP_STATIC){
		for (int i=0;i<6;i++)
			appearance.cube_map->fill_side(i, nix::LoadTexture(appearance.reflection_texture_file[i]));
		appearance.textures.add(appearance.cube_map);
	}
}





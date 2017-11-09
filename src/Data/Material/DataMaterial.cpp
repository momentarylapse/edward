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
	ed->makeDirs(filename);

	File *f = FileCreate(filename);
	f->WriteFileFormatVersion(false, 4);

	f->WriteComment("// Textures");
	f->WriteInt(appearance.texture_files.num);
	for (string &tf: appearance.texture_files)
		f->WriteStr(tf);
	f->WriteComment("// Colors");
	write_color_argb(f, appearance.ambient);
	write_color_argb(f, appearance.diffuse);
	write_color_argb(f, appearance.specular);
	f->WriteInt(appearance.shininess);
	write_color_argb(f, appearance.emissive);
	f->WriteComment("// Transparency");
	f->WriteInt(appearance.transparency_mode);
	f->WriteInt(appearance.alpha_factor * 100.0f);
	f->WriteInt(appearance.alpha_source);
	f->WriteInt(appearance.alpha_destination);
	f->WriteBool(appearance.alpha_z_buffer);
	f->WriteComment("// Appearance");
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteBool(false);
	f->WriteComment("// Reflection");
	f->WriteInt(appearance.reflection_mode);
	f->WriteInt(appearance.reflection_density);
	f->WriteInt(appearance.reflection_size);
	for (int i=0;i<6;i++)
		f->WriteStr(appearance.reflection_texture_file[i]);
	f->WriteComment("// ShaderFile");
	f->WriteStr(appearance.shader_file);
	f->WriteComment("// Physics");
	f->WriteInt(physics.friction_jump * 1000.0f);
	f->WriteInt(physics.friction_static * 1000.0f);
	f->WriteInt(physics.friction_sliding * 1000.0f);
	f->WriteInt(physics.friction_rolling * 1000.0f);
	f->WriteInt(physics.vmin_jump * 1000.0f);
	f->WriteInt(physics.vmin_sliding * 1000.0f);
	f->WriteComment("// Sound");
	f->WriteInt(Sound.NumRules);
	for (int i=0;i<Sound.NumRules;i++){
	}


	f->WriteStr("#");
	f->Close();
	delete(f);

	action_manager->markCurrentAsSave();
	return true;
}



bool DataMaterial::load(const string & _filename, bool deep)
{
	bool error=false;
	int ffv;
	reset();

	filename = _filename;
	ed->makeDirs(filename);
	File *f = FileOpen(filename);
	if (!f){
		ed->setMessage(_("Kann Material-Datei nicht &offnen"));
		return false;
	}
	file_time = f->GetDateModification().time;

	ffv=f->ReadFileFormatVersion();
	if (ffv<0){
		ed->errorBox(_("Datei-Format nicht ladbar!!"));
		error=true;
	}else if ((ffv == 3) or (ffv == 4)){
		if (ffv >= 4){
			f->ReadComment();
			int n = f->ReadInt();
			for (int i=0;i<n;i++)
				appearance.texture_files.add(f->ReadStr());
			if ((appearance.texture_files.num == 1) and (appearance.texture_files[0] == "")){
				appearance.texture_files.clear();
			}
		}
		// Colors
		f->ReadComment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = f->ReadInt();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->ReadComment();
		appearance.transparency_mode = f->ReadInt();
		appearance.alpha_factor = (float)f->ReadInt() * 0.01f;
		appearance.alpha_source = f->ReadInt();
		appearance.alpha_destination = f->ReadInt();
		appearance.alpha_z_buffer = f->ReadBool();
		// Appearance
		f->ReadComment();
		f->ReadInt();
		f->ReadInt();
		f->ReadBool();
		// Reflection
		f->ReadComment();
		appearance.reflection_mode = f->ReadInt();
		appearance.reflection_density = (float)f->ReadInt();
		appearance.reflection_size = f->ReadInt();
		for (int i=0;i<6;i++)
			appearance.reflection_texture_file[i] = f->ReadStr();
		// ShaderFile
		f->ReadComment();
		appearance.shader_file = f->ReadStr();
		// Physics
		f->ReadComment();
		physics.friction_jump = (float)f->ReadInt() * 0.001f;
		physics.friction_static = (float)f->ReadInt() * 0.001f;
		physics.friction_sliding = (float)f->ReadInt() * 0.001f;
		physics.friction_rolling = (float)f->ReadInt() * 0.001f;
		physics.vmin_jump = (float)f->ReadInt() * 0.001f;
		physics.vmin_sliding = (float)f->ReadInt() * 0.001f;
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->ReadIntC();
			Sound.NumRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)and(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->ReadComment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = (float)f->ReadInt();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->ReadComment();
		appearance.transparency_mode = f->ReadInt();
		appearance.alpha_factor = (float)f->ReadInt() * 0.01f;
		appearance.alpha_source = f->ReadInt();
		appearance.alpha_destination = f->ReadInt();
		// Appearance
		f->ReadComment();
		int MetalDensity = f->ReadInt();
		if (MetalDensity > 0){
			appearance.reflection_mode = REFLECTION_METAL;
			appearance.reflection_density = (float)MetalDensity;
		}
		f->ReadInt();
		f->ReadInt();
		bool Mirror = f->ReadBool();
		if (Mirror)
			appearance.reflection_mode = REFLECTION_MIRROR;
		f->ReadBool();
		// ShaderFile
		f->ReadComment();
		string sf = f->ReadStr();
		if (sf.num > 0)
			appearance.shader_file = sf + ".fx.glsl";
		// Physics
		f->ReadComment();
		physics.friction_jump = (float)f->ReadInt() * 0.001f;
		physics.friction_static = (float)f->ReadInt() * 0.001f;
		physics.friction_sliding = (float)f->ReadInt() * 0.001f;
		physics.friction_rolling = (float)f->ReadInt() * 0.001f;
		physics.vmin_jump = (float)f->ReadInt() * 0.001f;
		physics.vmin_sliding = (float)f->ReadInt() * 0.001f;

		appearance.alpha_z_buffer=(appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else if (ffv==1){
		// Colors
		f->ReadComment();
		read_color_argb(f, appearance.ambient);
		read_color_argb(f, appearance.diffuse);
		read_color_argb(f, appearance.specular);
		appearance.shininess = (float)f->ReadInt();
		read_color_argb(f, appearance.emissive);
		// Transparency
		f->ReadComment();
		appearance.transparency_mode = f->ReadInt();
		appearance.alpha_factor = (float)f->ReadInt() * 0.01f;
		appearance.alpha_source = f->ReadInt();
		appearance.alpha_destination = f->ReadInt();
		// Appearance
		f->ReadComment();
		int MetalDensity = f->ReadInt();
		if (MetalDensity > 0){
			appearance.reflection_mode = REFLECTION_METAL;
			appearance.reflection_density = (float)MetalDensity;
		}
		f->ReadInt();
		f->ReadInt();
		appearance.reflection_mode = (f->ReadBool() ? REFLECTION_MIRROR : REFLECTION_NONE);
		bool Mirror = f->ReadBool();
		if (Mirror)
			appearance.reflection_mode = REFLECTION_MIRROR;
		// ShaderFile
		f->ReadComment();
		string sf = f->ReadStr();
		if (sf.num > 0)
			appearance.shader_file = sf + ".fx.glsl";

		appearance.alpha_z_buffer = (appearance.transparency_mode != TRANSPARENCY_FUNCTIONS) and (appearance.transparency_mode != TRANSPARENCY_FACTOR);
	}else{
		ed->errorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 4));
		error = true;
	}

	f->Close();
	delete(f);


	if (deep)
		UpdateTextures();

	resetHistory();
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


	resetHistory();
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
			im.setPixel(i, j, color(1, f, f, f));
		}
	for (int i=0;i<6;i++)
		cube_map->overwrite_side(i, im);
}

void DataMaterial::UpdateTextures()
{
	msg_db_f("Mat.UpdateTextures", 1);
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





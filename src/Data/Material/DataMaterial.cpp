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

DataMaterial::~DataMaterial() {
	reset();
}




void DataMaterial::AppearanceData::reset() {
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

nix::Shader *DataMaterial::AppearanceData::get_shader() const {
	return nix::LoadShader(shader_file);
}


void DataMaterial::PhysicsData::Reset() {
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


void DataMaterial::SoundData::Reset() {
	NumRules = 0;
}

void DataMaterial::reset() {
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

void DataMaterial::ApplyForRendering() {
	nix::SetMaterial(appearance.ambient, appearance.diffuse, appearance.specular, appearance.shininess, appearance.emissive);

	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(true, true);
	if (appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_HARD) {
		nix::SetAlpha(ALPHA_COLOR_KEY_HARD);
	} else if (appearance.transparency_mode == TRANSPARENCY_COLOR_KEY_SMOOTH) {
		nix::SetAlpha(ALPHA_COLOR_KEY_SMOOTH);
	} else if (appearance.transparency_mode == TRANSPARENCY_FUNCTIONS) {
		nix::SetAlpha(appearance.alpha_source, appearance.alpha_destination);
		nix::SetZ(false, false);
	} else if (appearance.transparency_mode == TRANSPARENCY_FACTOR) {
		nix::SetAlpha(appearance.alpha_factor);
		nix::SetZ(false, false);
	}

	nix::SetShader(appearance.shader);

	nix::SetTextures(appearance.textures);
}

void create_fake_dynamic_cube_map(nix::CubeMap *cube_map) {
	Image im;
	int size = cube_map->width;
	im.create(size, size, White);
	for (int i=0; i<size; i++)
		for (int j=0; j<size; j++) {
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

void DataMaterial::UpdateTextures() {
	appearance.textures.clear();
	for (string &tf: appearance.texture_files)
		appearance.textures.add(nix::LoadTexture(tf));
	if (appearance.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC) {
		create_fake_dynamic_cube_map(appearance.cube_map);
		appearance.textures.add(appearance.cube_map);
	} else if (appearance.reflection_mode == REFLECTION_CUBE_MAP_STATIC) {
		for (int i=0;i<6;i++)
			appearance.cube_map->fill_side(i, nix::LoadTexture(appearance.reflection_texture_file[i]));
		appearance.textures.add(appearance.cube_map);
	}
}





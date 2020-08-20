/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "ShaderGraph.h"
#include "../../Storage/Storage.h"
#include "../../lib/nix/nix.h"



DataMaterial::DataMaterial() :
	Data(FD_MATERIAL)
{
	appearance.shader_graph = new ShaderGraph();
	reset();
}

DataMaterial::~DataMaterial() {
	reset();
	delete appearance.shader_graph;
}


void DataMaterial::AppearanceData::reset() {
	texture_files.clear();

	ambient = 0.5f;
	diffuse = White;
	specular = 0;
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


	shader_file = "";
	shader_graph->make_default();
	shader_code = shader_graph->build_source();
	shader_from_graph = true;
	is_default_shader = true;
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

	appearance.reset();
	physics.Reset();
	Sound.Reset();


	reset_history();
	notify();
}

void DataMaterial::apply_for_rendering() {
	nix::SetMaterial(appearance.diffuse, appearance.ambient, appearance.specular, appearance.shininess, appearance.emissive);

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
}

void DataMaterial::AppearanceData::update_shader_from_file() {
	if (shader_file != "") {
		shader_code = FileReadText(nix::shader_dir << shader_file);
		if (file_exists(nix::shader_dir << shader_file.with(".graph"))) {
			shader_graph->load(nix::shader_dir << shader_file.with(".graph"));
			shader_code = shader_graph->build_source();
			shader_from_graph = true;
		} else {
			shader_from_graph = false;
		}
		is_default_shader = false;
	} else {
		shader_graph->make_default();
		shader_code = shader_graph->build_source();
		shader_from_graph = true;
		is_default_shader = true;
	}
}






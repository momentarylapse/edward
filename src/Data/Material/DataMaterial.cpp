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



DataMaterial::DataMaterial(bool with_graph) :
	Data(FD_MATERIAL)
{
	shader.graph = nullptr;
	if (with_graph)
		shader.graph = new ShaderGraph();
	reset();
}

DataMaterial::~DataMaterial() {
	reset();
	if (shader.graph)
		delete shader.graph;
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
	reflection_texture_file.clear();
	reflection_texture_file.resize(6);
}

void DataMaterial::ShaderData::reset() {
	set_engine_default();
}


void DataMaterial::PhysicsData::reset() {
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


void DataMaterial::SoundData::reset() {
	NumRules = 0;
}

void DataMaterial::reset() {
	filename = "";

	appearance.reset();
	shader.reset();
	physics.reset();
	Sound.reset();


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

void DataMaterial::ShaderData::load_from_file() {
	if (file.is_empty()) {
		set_engine_default();
		return;
	}
	code = FileReadText(nix::shader_dir << file);
	if (file_exists(nix::shader_dir << file.with(".graph"))) {
		graph->load(nix::shader_dir << file.with(".graph"));
		code = graph->build_source();
		from_graph = true;
	} else {
		from_graph = false;
	}
	is_default = false;
}

void DataMaterial::ShaderData::set_engine_default() {
	file = "";
	code = "";
	if (graph) {
		graph->make_default_for_engine();
		code = graph->build_source();
	}
	from_graph = true;
	is_default = true;
}

void DataMaterial::ShaderData::save_to_file() {
	if (!file.is_empty()) {
		code = graph->build_source();
		graph->save(nix::shader_dir << file.with(".graph"));
		from_graph = true;
		is_default= false;
	}
}






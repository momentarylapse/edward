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
#include "../../y/ResourceManager.h"



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

	albedo = White;
	roughness = 0.5f;
	metal = 0;
	emissive = Black;

	transparency_mode = TransparencyMode::NONE;
	alpha_source = alpha_destination = nix::Alpha::ZERO;
	alpha_factor = 0.5f;
	alpha_z_buffer = true;

	reflection_mode = ReflectionMode::NONE;
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
	nix::set_material(appearance.albedo, appearance.roughness, appearance.metal, appearance.emissive);

	nix::disable_alpha();
	nix::set_z(true, true);
	if (appearance.transparency_mode == TransparencyMode::COLOR_KEY_HARD) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (appearance.transparency_mode == TransparencyMode::COLOR_KEY_SMOOTH) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (appearance.transparency_mode == TransparencyMode::FUNCTIONS) {
		nix::set_alpha(appearance.alpha_source, appearance.alpha_destination);
		nix::set_z(false, false);
	} else if (appearance.transparency_mode == TransparencyMode::FACTOR) {
		//nix::set_alpha(appearance.alpha_factor);
		nix::set_z(false, false);
	}
}

void DataMaterial::ShaderData::load_from_file() {
	if (file.is_empty()) {
		set_engine_default();
		return;
	}
	if (file_exists(ResourceManager::shader_dir << file)) {
		code = FileReadText(ResourceManager::shader_dir << file);
	}
	if (file_exists(ResourceManager::shader_dir << file.with(".graph"))) {
		graph->load(ResourceManager::shader_dir << file.with(".graph"));
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
#ifdef OS_WINDOWS
		msg_error("TODO  DataMaterial::ShaderData::set_engine_default()");
#else
		graph->make_default_for_engine();
		code = graph->build_source();
#endif
	}
	from_graph = true;
	is_default = true;
}

void DataMaterial::ShaderData::save_to_file() {
	if (file) {
		code = graph->build_source();
		FileWriteText(ResourceManager::shader_dir << file, code);
		graph->save(ResourceManager::shader_dir << file.with(".graph"));
		from_graph = true;
		is_default = false;
	}
}






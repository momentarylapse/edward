/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "ShaderGraph.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/file.h"
#include "../../lib/os/msg.h"
#include <y/helper/ResourceManager.h>



DataMaterial::DataMaterial(Session *s) :
	Data(s, FD_MATERIAL)
{
	reset();
}

DataMaterial::~DataMaterial() {
	for (auto &p: appearance.passes)
		if (p.shader.graph)
			delete p.shader.graph;
}


void DataMaterial::AppearanceData::reset(Session *session) {
	texture_files.clear();

	albedo = White;
	roughness = 0.5f;
	metal = 0;
	emissive = Black;


	for (auto &p: passes) {
		if (p.shader.graph)
			delete p.shader.graph;
		p.shader.graph = nullptr;
	}
	passes.clear();

	passes.resize(1);
	passes[0].shader.reset(session);
}

void DataMaterial::ShaderData::reset(Session *s) {
	if (!graph)
		graph = new ShaderGraph(s);
	set_engine_default(s);
}


void DataMaterial::PhysicsData::reset() {
	friction_jump = 0.7f;
	friction_static = 0.8f;
	friction_sliding = 0.5f;
	friction_rolling = 0.2f;
	vmin_jump = 10;
	vmin_sliding = 10;
}

void DataMaterial::reset() {
	filename = "";

	appearance.reset(session);
	physics.reset();


	reset_history();
	out_changed();
}

void DataMaterial::apply_for_rendering(int pass_no) const {
#if HAS_LIB_GL
	nix::set_material(appearance.albedo, appearance.roughness, appearance.metal, appearance.emissive);

	nix::disable_alpha();
	nix::set_z(true, true);
	auto &p = appearance.passes[pass_no];
	if (p.mode == TransparencyMode::COLOR_KEY_HARD) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (p.mode == TransparencyMode::COLOR_KEY_SMOOTH) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (p.mode == TransparencyMode::MIX) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
		nix::set_z(false, true);
	} else if (p.mode == TransparencyMode::FUNCTIONS) {
		nix::set_alpha(p.source, p.destination);
		nix::set_z(false, true);
	} else if (p.mode == TransparencyMode::FACTOR) {
		//nix::set_alpha(appearance.alpha_factor);
		nix::set_z(false, true);
	}
	nix::set_cull(p.culling);
}

void DataMaterial::ShaderData::load_from_file(Session *s) {
	if (file.is_empty()) {
		set_engine_default(s);
		return;
	}
	if (os::fs::exists(s->resource_manager->shader_dir | file)) {
		code = os::fs::read_text(s->resource_manager->shader_dir | file);
	}
	if (os::fs::exists(s->resource_manager->shader_dir | file.with(".graph"))) {
		graph->load(s->resource_manager->shader_dir | file.with(".graph"));
		code = graph->build_source();
		from_graph = true;
	} else {
		from_graph = false;
	}
	is_default = false;
#endif
}

void DataMaterial::ShaderData::set_engine_default(Session *s) {
	file = "";
#ifdef OS_WINDOWS
	msg_error("TODO  DataMaterial::ShaderData::set_engine_default()");
	code = ShaderGraph::build_default_source();
#else
	graph->make_default_for_engine();
	code = graph->build_source();
#endif
	from_graph = true;
	is_default = true;
}

void DataMaterial::ShaderData::save_to_file(Session *s) {
	if (file) {
		code = graph->build_source();
		msg_write("saving " + str(s->resource_manager->shader_dir | file));
		os::fs::write_text(s->resource_manager->shader_dir | file, code);
		graph->save(s->resource_manager->shader_dir | file.with(".graph"));
		from_graph = true;
		is_default = false;
	}
}






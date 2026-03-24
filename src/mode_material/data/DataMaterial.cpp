/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
//#include "ShaderGraph.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/file.h"
#include "../../lib/os/msg.h"
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/TextureManager.h>

#include "view/DocumentSession.h"


DataMaterial::DataMaterial(DocumentSession* d) :
	Data(d, FD_MATERIAL)
{
	reset();
}

DataMaterial::~DataMaterial() {
#if ksdjfhskdjfh
	for (auto &p: appearance.passes)
		if (p.shader.graph)
			delete p.shader.graph;
#endif
}



void DataMaterial::ShaderData::reset(DocumentSession *s) {
#if ksdjfhskdjfh
	if (!graph)
		graph = new ShaderGraph(s);
#endif
	set_engine_default(s);
}



void DataMaterial::reset() {
	filename = "";

	material.textures = {session->resource_manager->load_texture("")};

	material.albedo = White;
	material.roughness = 0.5f;
	material.metal = 0;
	material.emission = Black;

	material.cast_shadow = true;

#if ksdjfhskdjfh
	for (auto &p: passes) {
		if (p.shader.graph)
			delete p.shader.graph;
		p.shader.graph = nullptr;
	}
#endif

	material.set_num_passes(1);
	material.pass(0) = yrenderer::Material::RenderPassData();
	//material.pass(0).shader_path = "";

	material.friction.jump = 0.7f;
	material.friction._static = 0.8f;
	material.friction.sliding = 0.5f;
	material.friction.rolling = 0.2f;
//	material.friction.vmin_jump = 10;
//	material.friction.vmin_sliding = 10;


	reset_history();
	out_changed();
}

void DataMaterial::apply_for_rendering(int pass_no) const {
#if HAS_LIB_GL
	nix::set_material(material.albedo, material.roughness, material.metal, material.emission);

	nix::disable_alpha();
	nix::set_z(true, true);
	auto &p = material.pass(pass_no);
	if (p.mode == yrenderer::TransparencyMode::COLOR_KEY_HARD) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (p.mode == yrenderer::TransparencyMode::COLOR_KEY_SMOOTH) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	} else if (p.mode == yrenderer::TransparencyMode::MIX) {
		nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
		nix::set_z(false, true);
	} else if (p.mode == yrenderer::TransparencyMode::FUNCTIONS) {
		nix::set_alpha(p.source, p.destination);
		nix::set_z(false, true);
	} else if (p.mode == yrenderer::TransparencyMode::FACTOR) {
		//nix::set_alpha(appearance.alpha_factor);
		nix::set_z(false, true);
	}
	nix::set_cull(p.cull_mode);
#endif
}

void DataMaterial::ShaderData::load_from_file(DocumentSession *s) {
#if ksdjfhskdjfh
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

void DataMaterial::ShaderData::set_engine_default(DocumentSession *s) {
	file = "";
#if ksdjfhskdjfh
#ifdef OS_WINDOWS
	msg_error("TODO  DataMaterial::ShaderData::set_engine_default()");
	code = ShaderGraph::build_default_source();
#else
	graph->make_default_for_engine();
	code = graph->build_source();
#endif
#endif
	from_graph = true;
	is_default = true;
}

void DataMaterial::ShaderData::save_to_file(DocumentSession *s) {
#if ksdjfhskdjfh
	if (file) {
		code = graph->build_source();
		msg_write("saving " + str(s->resource_manager->shader_dir | file));
		os::fs::write_text(s->resource_manager->shader_dir | file, code);
		graph->save(s->resource_manager->shader_dir | file.with(".graph"));
		from_graph = true;
		is_default = false;
	}
#endif
}



DataMaterial DataMaterial::from_material(DocumentSession* s, yrenderer::Material *material) {
	DataMaterial m(s);
	m.material = *material;
	return m;
}


const yrenderer::Material* DataMaterial::to_material() const {
	return &material;
}






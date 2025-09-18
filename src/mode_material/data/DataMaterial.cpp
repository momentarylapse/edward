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


void DataMaterial::AppearanceData::reset(DocumentSession *session) {
	texture_files = {""};

	albedo = White;
	roughness = 0.5f;
	metal = 0;
	emissive = Black;

	cast_shadow = true;

#if ksdjfhskdjfh
	for (auto &p: passes) {
		if (p.shader.graph)
			delete p.shader.graph;
		p.shader.graph = nullptr;
	}
#endif
	passes.clear();

	passes.resize(1);
	passes[0].shader.reset(session);
}

void DataMaterial::ShaderData::reset(DocumentSession *s) {
#if ksdjfhskdjfh
	if (!graph)
		graph = new ShaderGraph(s);
#endif
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

	appearance.reset(doc);
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
	nix::set_cull(p.culling);
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
	m.appearance.albedo = material->albedo;
	m.appearance.emissive = material->emission;
	m.appearance.metal = material->metal;
	m.appearance.roughness = material->roughness;
	m.appearance.texture_files.clear();
	const Path dir = s->session->storage->get_root_dir(FD_TEXTURE);
	for (int i=0;i<material->textures.num;i++)
		m.appearance.texture_files.add(s->session->resource_manager->texture_manager->texture_file(material->textures[i].get()).relative_to(dir));
	m.appearance.passes[0].shader.file = material->pass0.shader_path;

	// TODO alpha etc

	m.physics.friction_jump = material->friction.jump;
	m.physics.friction_rolling = material->friction.rolling;
	m.physics.friction_static = material->friction._static;
	m.physics.friction_sliding = material->friction.sliding;
	return m;
}


yrenderer::Material* DataMaterial::to_material() const {
	auto m = new yrenderer::Material(session->ctx);
	m->albedo = appearance.albedo;
	m->roughness = appearance.roughness;
	m->metal = appearance.metal;
	m->emission = appearance.emissive;
	m->cast_shadow = appearance.cast_shadow;

	for (const auto t: appearance.texture_files)
		m->textures.add(session->resource_manager->load_texture(t));

	m->num_passes = appearance.passes.num;
	if (m->num_passes >= 2)
		m->extended = new yrenderer::Material::ExtendedData;
	for (int i=0; i<appearance.passes.num; i++) {
		auto &p = appearance.passes[i];
		auto &pp = m->pass(i);
		pp.mode = p.mode;
		pp.source = p.source;
		pp.destination = p.destination;
		pp.z_buffer = p.z_write;
		pp.z_test = p.z_test;
		pp.cull_mode = p.culling;
		pp.shader_path = p.shader.file;
	}

	return m;
}






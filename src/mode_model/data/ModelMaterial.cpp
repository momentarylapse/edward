/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../lib/image/image.h"
#if 0 //HAS_LIB_GL
#include "../../multiview/Window.h"
#include "../../multiview/DrawingHelper.h"
#endif
#include <renderer/world/WorldRenderer.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/ygraphics/graphics-impl.h>
#include "../../Session.h"

namespace yrenderer {
	float col_frac(const color &a, const color &b);
}

ModelMaterial::ModelMaterial(Session *_s) {
	session = _s;
	// default file
	material = session->resource_manager->load_material("");

	// color
	col.user = false;
	load_colors_from_file();
}


// ONLY ActionModelAddMaterial
ModelMaterial::ModelMaterial(Session *_s, const Path &_filename) : ModelMaterial(_s) {
	filename = _filename;
	make_consistent_after_shallow_loading();
}

ModelMaterial::~ModelMaterial() = default;

void ModelMaterial::TextureLevel::reload_image(Session *session) {
	if (filename == "")
		image = new Image(512, 512, White);
	else
		image = Image::load(session->resource_manager->texture_manager->texture_dir | filename);
	edited = false;
	if (session->resource_manager->ctx)
		update_texture();
}

void ModelMaterial::TextureLevel::update_texture() {
	if (!texture)
		texture = new ygfx::Texture();
	texture->write(*image);
}


color ModelMaterial::Color::ambient() const {
	return albedo * roughness * 0.5f;
}

color ModelMaterial::Color::specular() const {
	return White * (1 - roughness);
}

float ModelMaterial::Color::shininess() const {
	return 5 / (1.1f - roughness);
}

void ModelMaterial::Color::import(const color &am, const color &di, const color &sp, float shininess, const color &em) {
	albedo = di;
	emission = em;
	roughness = yrenderer::col_frac(am, di) / 2;
	metal = 0;
}

void ModelMaterial::make_consistent_after_shallow_loading() {
	material = session->resource_manager->load_material(filename);

	if (material->reflection.mode == yrenderer::ReflectionMode::CUBE_MAP_DYNAMIC) {
	//	if (!material->cube_map)
	//		material->cube_map = new nix::CubeMap(material->cube_map_size);
	//	create_fake_dynamic_cube_map(material->cube_map);
	}

	check_textures();
	if (!col.user)
		load_colors_from_file();
}



void ModelMaterial::check_textures() {

	//msg_write("--------Mat.check textures()");
	// parent has more texture levels?
//	if (material->textures.num > texture_levels.num) {
//		while (material->textures.num > texture_levels.num)
//			texture_levels.add(new ModelMaterial::TextureLevel);
//		session->set_message(_("Number of textures changed to comply with the material!"));
//	}

	// load all textures
	foreachi (auto t, weak(texture_levels), i) {
		/*auto *prev = t->texture;
		t->texture = nix::LoadTexture(t->filename);

		// parent overwrites unused textures
		if (i < material->textures.num)
			if (t->filename == "")
				t->texture = material->textures[i];

		if (t->texture != prev) {
			delete t->image;
			if (t->filename == "")
				t->image = new Image(16, 16, White);
			else
				t->image = Image::load(t->filename);
		}*/
		t->reload_image(session);
	}
}

void ModelMaterial::load_colors_from_file() {
	col.albedo = material->albedo;
	col.roughness = material->roughness;
	col.metal = material->metal;
	col.emission = material->emission;
}

#if 0
void ModelMaterial::apply_for_rendering(MultiView::Window *w) {
#if HAS_LIB_GL
	nix::disable_alpha();
	w->set_shader(w->ctx->default_3d.get());
	color em = color::interpolate(col.emission, White, 0.1f);
	nix::set_material(col.albedo, col.roughness, col.metal, em);
	nix::set_z(true, true);
	if (true) {//MVFXEnabled){
		//nix::set_z(alpha.zbuffer, alpha.zbuffer);
		auto &p = material->pass0;
		auto mode = p.mode;
		auto source = p.source;
		auto dest = p.destination;
		if (p.mode == TransparencyMode::DEFAULT) {
			mode = material->pass0.mode;
			source = material->pass0.source;
			dest = material->pass0.destination;
		}
		if (mode == TransparencyMode::COLOR_KEY_HARD) {
			nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
		} else if (mode == TransparencyMode::COLOR_KEY_SMOOTH) {
			nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
		} else if (mode == TransparencyMode::FUNCTIONS) {
			nix::set_alpha(source, dest);
			nix::set_z(false, true);
		} else if (mode == TransparencyMode::FACTOR) {
			//nix::set_alpha(alpha.factor);
			nix::set_z(false, true);
		}
		shader_cache._prepare_shader(RenderPathType::FORWARD, *material, "default", "");
		w->set_shader(shader_cache.shader[0].get());
	}
	Array<nix::Texture*> tex;
	for (auto &t: texture_levels)
		tex.add(t.texture.get());
	while(tex.num < 5)
		tex.add(session->drawing_helper->tex_white.get());
	tex.add(MultiView::cube_map.get());
//	if (material->cube_map)
//		tex.add(material->cube_map);
	nix::bind_textures(tex);
#endif
}
#endif


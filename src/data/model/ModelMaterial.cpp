/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../multiview/Window.h"
#include "../../multiview/DrawingHelper.h"
#include "../../y/ResourceManager.h"
#include "../../EdwardWindow.h"

float col_frac(const color &a, const color &b);


ModelMaterial::ModelMaterial() {
	// file
	filename = "";
	material = LoadMaterial("");

	// color
	col.user = false;
	check_colors();

	// transparency
	alpha.mode = TransparencyMode::DEFAULT;
	alpha.destination = nix::Alpha::ZERO;
	alpha.source = nix::Alpha::ZERO;
	alpha.factor = 50;
	alpha.zbuffer = true;
	check_transparency();

	vb = nullptr;
}


// ONLY ActionModelAddMaterial
ModelMaterial::ModelMaterial(const Path &_filename) : ModelMaterial() {
	filename = _filename;
	make_consistent();
}

ModelMaterial::~ModelMaterial() {
	if (vb)
		delete vb;
}

ModelMaterial::TextureLevel::TextureLevel() {
	image = nullptr;
	edited = false;
}

ModelMaterial::TextureLevel::~TextureLevel() {
	if (image)
		delete image;
}

void ModelMaterial::TextureLevel::reload_image() {
	if (image)
		delete image;
	if (filename == "")
		image = new Image(512, 512, White);
	else
		image = Image::load(ResourceManager::texture_dir | filename);
	edited = false;
	update_texture();
}

void ModelMaterial::TextureLevel::update_texture() {
	if (!texture)
		texture = new nix::Texture();
	texture->write(*image);
}

// DEPRECATED
void ModelMaterial::reset() {
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
	roughness = col_frac(am, di) / 2;
	metal = 0;
}

void ModelMaterial::make_consistent() {
	material = LoadMaterial(filename);

	if (material->reflection.mode == ReflectionMode::CUBE_MAP_DYNAMIC) {
	//	if (!material->cube_map)
	//		material->cube_map = new nix::CubeMap(material->cube_map_size);
	//	create_fake_dynamic_cube_map(material->cube_map);
	}

	check_textures();
	check_transparency();
	check_colors();
}

bool ModelMaterial::Alpha::user_defined() const {
	return mode != TransparencyMode::DEFAULT;
}

void ModelMaterial::check_transparency() {
	if (!alpha.user_defined()) {
		//alpha.mode = material->alpha.mode;
		alpha.source = material->alpha.source;
		alpha.destination = material->alpha.destination;
		alpha.factor	= material->alpha.factor;
		alpha.zbuffer = material->alpha.z_buffer;
	}
}



void ModelMaterial::check_textures() {

	//msg_write("--------Mat.check textures()");
	// parent has more texture levels?
//	if (material->textures.num > texture_levels.num) {
//		while (material->textures.num > texture_levels.num)
//			texture_levels.add(new ModelMaterial::TextureLevel);
//		ed->set_message(_("Number of textures changed to comply with the material!"));
//	}

	// load all textures
	foreachi (auto *t, texture_levels, i) {
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
		t->reload_image();
	}
}

void ModelMaterial::check_colors() {
	if (!col.user) {
		col.albedo = material->albedo;
		col.roughness = material->roughness;
		col.metal = material->metal;
		col.emission = material->emission;
	}
}

void ModelMaterial::apply_for_rendering(MultiView::Window *w) {
	nix::disable_alpha();
	w->set_shader(nix::Shader::default_3d.get());
	color em = color::interpolate(col.emission, White, 0.1f);
	nix::set_material(col.albedo, col.roughness, col.metal, em);
	nix::set_z(true, true);
	if (true) {//MVFXEnabled){
		//nix::set_z(alpha.zbuffer, alpha.zbuffer);
		auto mode = alpha.mode;
		auto source = alpha.source;
		auto dest = alpha.destination;
		if (alpha.mode == TransparencyMode::DEFAULT) {
			mode = material->alpha.mode;
			source = material->alpha.source;
			dest = material->alpha.destination;
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
		material->_prepare_shader((RenderPathType)1, ShaderVariant::DEFAULT);
		w->set_shader(material->shader[0].get());
	}
	Array<nix::Texture*> tex;
	for (auto *t: texture_levels)
		tex.add(t->texture.get());
	tex.resize(5);
	tex.add(MultiView::cube_map.get());
//	if (material->cube_map)
//		tex.add(material->cube_map);
	nix::set_textures(tex);
}


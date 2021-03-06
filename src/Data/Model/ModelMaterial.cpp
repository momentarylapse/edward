/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../MultiView/Window.h"
#include "../../MultiView/DrawingHelper.h"
#include "../../y/ResourceManager.h"
#include "../../Edward.h"

float col_frac(const color &a, const color &b);


ModelMaterial::ModelMaterial() {
	// file
	filename = "";
	material = LoadMaterial("");

	// color
	col.user = false;
	check_colors();

	// transparency
	alpha.user = false;
	check_transparency();
	alpha.mode = TransparencyMode::DEFAULT;
	alpha.destination = nix::Alpha::ZERO;
	alpha.source = nix::Alpha::ZERO;
	alpha.factor = 50;
	alpha.zbuffer = true;

	vb = NULL;
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
		image = Image::load(ResourceManager::texture_dir << filename);
	edited = false;
	update_texture();
}

void ModelMaterial::TextureLevel::update_texture() {
	if (!texture)
		texture = new nix::Texture();
	texture->overwrite(*image);
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

void ModelMaterial::check_transparency() {
	if (alpha.mode == TransparencyMode::DEFAULT)
		alpha.user = false;
	if (!alpha.user) {
		alpha.mode = material->alpha.mode;
		alpha.source = material->alpha.source;
		alpha.destination = material->alpha.destination;
		alpha.factor	= material->alpha.factor;
		alpha.zbuffer = material->alpha.z_buffer;
	}
}



void ModelMaterial::check_textures() {

	msg_write("--------Mat.check textures()");
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
	nix::set_alpha(nix::AlphaMode::NONE);
	w->set_shader(nix::Shader::default_3d);
	color em = color::interpolate(col.emission, White, 0.1f);
	nix::set_material(col.albedo, col.roughness, col.metal, em);
	if (true) {//MVFXEnabled){
		nix::set_z(alpha.zbuffer, alpha.zbuffer);
		if (alpha.mode == TransparencyMode::COLOR_KEY_HARD) {
			nix::set_alpha(nix::AlphaMode::COLOR_KEY_HARD);
		} else if (alpha.mode == TransparencyMode::COLOR_KEY_SMOOTH) {
			nix::set_alpha(nix::AlphaMode::COLOR_KEY_SMOOTH);
		} else if (alpha.mode == TransparencyMode::FUNCTIONS) {
			nix::set_alpha(alpha.source, alpha.destination);
			//NixSetZ(false,false);
		} else if (alpha.mode == TransparencyMode::FACTOR) {
			//nix::set_alpha(alpha.factor);
			//NixSetZ(false,false);
		}
		material->prepare_shader(ShaderVariant::DEFAULT);
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


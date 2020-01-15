/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../Edward.h"

void create_fake_dynamic_cube_map(nix::CubeMap *cube_map); // DataMaterial.cpp


ModelMaterial::ModelMaterial() {
	// file
	filename = "";
	material = LoadMaterial("");

	// color
	col.user = false;
	checkColors();

	// transparency
	alpha.user = false;
	checkTransparency();
	alpha.mode = TRANSPARENCY_DEFAULT;
	alpha.destination = 0;
	alpha.source = 0;
	alpha.factor = 50;
	alpha.zbuffer = true;

	vb = NULL;
}


// ONLY ActionModelAddMaterial
ModelMaterial::ModelMaterial(const string &_filename) : ModelMaterial() {
	filename = _filename;
	makeConsistent();
}

ModelMaterial::~ModelMaterial()
{
	if (vb)
		delete(vb);
	vb = NULL;
}

ModelMaterial::TextureLevel::TextureLevel() {
	texture = NULL;
	image = NULL;
	edited = false;
}

ModelMaterial::TextureLevel::~TextureLevel() {
	if (image)
		delete image;
	if (texture)
		delete texture;
}

void ModelMaterial::TextureLevel::reload_image() {
	if (image)
		delete image;
	if (filename == "")
		image = new Image(32, 32, White);
	else
		image = Image::load(nix::texture_dir + filename);
	edited = false;
	update_texture();
}

void ModelMaterial::TextureLevel::update_texture() {
	if (!texture)
		texture = new nix::Texture();
	texture->overwrite(*image);
}

// DEPRECATED
void ModelMaterial::reset()
{
}


void ModelMaterial::makeConsistent()
{
	material = LoadMaterial(filename);

	if (material->reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC){
		if (!material->cube_map)
			material->cube_map = new nix::CubeMap(material->cube_map_size);
		create_fake_dynamic_cube_map(material->cube_map);
	}

	checkTextures();
	checkTransparency();
	checkColors();
}

void ModelMaterial::checkTransparency()
{
	if (alpha.mode == TRANSPARENCY_DEFAULT)
		alpha.user = false;
	if (!alpha.user){
		alpha.mode = material->transparency_mode;
		alpha.source = material->alpha_source;
		alpha.destination = material->alpha_destination;
		alpha.factor	= material->alpha_factor;
		alpha.zbuffer = material->alpha_z_buffer;
	}
}



void ModelMaterial::checkTextures()
{

	msg_write("--------Mat.check textures()");
	// parent has more texture levels?
	if (material->textures.num > texture_levels.num){
		texture_levels.resize(material->textures.num);
		ed->set_message(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}

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

void ModelMaterial::checkColors()
{
	if (!col.user){
		col.ambient = material->ambient;
		col.diffuse = material->diffuse;
		col.specular = material->specular;
		col.shininess = material->shininess;
		col.emission = material->emission;
	}
}

void ModelMaterial::applyForRendering()
{
	nix::SetAlpha(ALPHA_NONE);
	nix::SetShader(nix::default_shader_3d);
	color em = ColorInterpolate(col.emission, White, 0.1f);
	nix::SetMaterial(col.ambient, col.diffuse, col.specular, col.shininess, em);
	if (true){//MVFXEnabled){
		nix::SetZ(alpha.zbuffer, alpha.zbuffer);
		if (alpha.mode == TRANSPARENCY_COLOR_KEY_HARD)
			nix::SetAlpha(ALPHA_COLOR_KEY_HARD);
		else if (alpha.mode == TRANSPARENCY_COLOR_KEY_SMOOTH)
			nix::SetAlpha(ALPHA_COLOR_KEY_SMOOTH);
		else if (alpha.mode == TRANSPARENCY_FUNCTIONS){
			nix::SetAlpha(alpha.source, alpha.destination);
			//NixSetZ(false,false);
		}else if (alpha.mode == TRANSPARENCY_FACTOR){
			nix::SetAlpha(alpha.factor);
			//NixSetZ(false,false);
		}
		nix::SetShader(material->shader);
	}
	Array<nix::Texture*> tex;
	for (auto *t: texture_levels)
		tex.add(t->texture);
	if (material->cube_map)
		tex.add(material->cube_map);
	nix::SetTextures(tex);
}


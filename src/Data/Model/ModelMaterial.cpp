/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../Edward.h"


ModelMaterial::ModelMaterial()
{
	vb = NULL;
	reset();
}

ModelMaterial::ModelMaterial(const string &filename)
{
	vb = NULL;
	reset();
	material_file = filename;
	makeConsistent();
}

ModelMaterial::~ModelMaterial()
{
	if (vb)
		delete(vb);
	vb = NULL;
}

void ModelMaterial::reset()
{
	// transparency
	user_transparency = false;
	transparency_mode = TransparencyModeDefault;
	alpha_destination = 0;
	alpha_source = 0;
	alpha_factor = 50;
	alpha_zbuffer = true;

	// color
	user_color = false;
	ambient = White;
	diffuse = White;
	specular = Black;
	emission = Black;
	shininess = 20;

	// file
	material_file = "";
	material = LoadMaterial("");

	// textures
	num_textures = 1;
	texture_file[0] = "";
	texture[0] = NULL;

	if (vb)
		delete(vb);
	vb = NULL;
}

void ModelMaterial::operator =(const ModelMaterial &m)
{
	user_transparency = m.user_transparency;
	transparency_mode = m.transparency_mode;
	alpha_destination = m.alpha_destination;
	alpha_source = m.alpha_source;
	alpha_factor = m.alpha_factor;
	alpha_zbuffer = m.alpha_zbuffer;

	user_color = m.user_color;
	ambient = m.ambient;
	diffuse = m.diffuse;
	specular = m.specular;
	emission = m.emission;
	shininess = m.shininess;

	material_file = m.material_file;
	material = m.material;

	num_textures = m.num_textures;
	for (int i=0; i<MATERIAL_MAX_TEXTURES; i++){
		texture_file[i] = m.texture_file[i];
		texture[i] = m.texture[i];
	}
}

void ModelMaterial::makeConsistent()
{
	material = LoadMaterial(material_file);
	checkTextures();
	checkTransparency();
	checkColors();
}

void ModelMaterial::checkTransparency()
{
	if (transparency_mode == TransparencyModeDefault)
		user_transparency = false;
	if (!user_transparency){
		transparency_mode = material->transparency_mode;
		alpha_source = material->alpha_source;
		alpha_destination = material->alpha_destination;
		alpha_factor	= material->alpha_factor;
		alpha_zbuffer = material->alpha_z_buffer;
	}
}



void ModelMaterial::checkTextures()
{
	// parent has more texture levels?
	if (material->textures.num > num_textures){
		for (int i=num_textures;i<material->textures.num;i++){
			texture_file[i] = "";
			texture[i] = NULL;
		}
		num_textures = material->textures.num;
		ed->setMessage(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}

	// load all textures
	for (int i=0;i<num_textures;i++)
		texture[i] = nix::LoadTexture(texture_file[i]);

	// parent overwrites unused textures
	for (int i=0;i<material->textures.num;i++)
		if (texture[i] < 0)
			if (texture_file[i].num == 0)
				texture[i] = material->textures[i];
}

void ModelMaterial::checkColors()
{
	if (!user_color){
		ambient = material->ambient;
		diffuse = material->diffuse;
		specular = material->specular;
		shininess = material->shininess;
		emission = material->emission;
	}
}

void ModelMaterial::applyForRendering()
{
	nix::SetAlpha(AlphaNone);
	nix::SetShader(NULL);
	color em = ColorInterpolate(emission, White, 0.1f);
	nix::SetMaterial(ambient, diffuse, specular, shininess, em);
	if (true){//MVFXEnabled){
		nix::SetZ(alpha_zbuffer, alpha_zbuffer);
		if (transparency_mode == TransparencyModeColorKeyHard)
			nix::SetAlpha(AlphaColorKeyHard);
		else if (transparency_mode == TransparencyModeColorKeySmooth)
			nix::SetAlpha(AlphaColorKeySmooth);
		else if (transparency_mode == TransparencyModeFunctions){
			nix::SetAlpha(alpha_source, alpha_destination);
			//NixSetZ(false,false);
		}else if (transparency_mode == TransparencyModeFactor){
			nix::SetAlpha(alpha_factor);
			//NixSetZ(false,false);
		}
		nix::SetShader(material->shader);
	}
	Array<nix::Texture*> tex;
	if (material->cube_map >= 0){
		// evil hack
		tex.add(texture[0]);
		tex.add(texture[1]);
		tex.add(texture[2]);
		tex.add(material->cube_map);
		nix::SetTextures(tex);
	}else{
		nix::SetTextures(material->textures);
	}
}


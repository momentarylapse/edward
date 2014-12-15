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
{	reset();	}

ModelMaterial::ModelMaterial(const string &filename)
{
	reset();
	material_file = filename;
	MakeConsistent();
}

ModelMaterial::~ModelMaterial()
{}

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
}

void ModelMaterial::MakeConsistent()
{
	material = LoadMaterial(material_file);
	CheckTextures();
	CheckTransparency();
	CheckColors();
}

void ModelMaterial::CheckTransparency()
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



void ModelMaterial::CheckTextures()
{
	// parent has more texture levels?
	if (material->num_textures > num_textures){
		for (int i=num_textures;i<material->num_textures;i++){
			texture_file[i] = "";
			texture[i] = NULL;
		}
		num_textures = material->num_textures;
		ed->setMessage(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}

	// load all textures
	for (int i=0;i<num_textures;i++)
		texture[i] = NixLoadTexture(texture_file[i]);

	// parent overwrites unused textures
	for (int i=0;i<material->num_textures;i++)
		if (texture[i] < 0)
			if (texture_file[i].num == 0)
				texture[i] = material->texture[i];
}

void ModelMaterial::CheckColors()
{
	if (!user_color){
		ambient = material->ambient;
		diffuse = material->diffuse;
		specular = material->specular;
		shininess = material->shininess;
		emission = material->emission;
	}
}

void ModelMaterial::ApplyForRendering()
{
	NixSetAlpha(AlphaNone);
	NixSetShader(NULL);
	color em = ColorInterpolate(emission, White, 0.1f);
	NixSetMaterial(ambient, diffuse, specular, shininess, em);
	if (true){//MVFXEnabled){
		NixSetZ(alpha_zbuffer, alpha_zbuffer);
		if (transparency_mode == TransparencyModeColorKeyHard)
			NixSetAlpha(AlphaColorKeyHard);
		else if (transparency_mode == TransparencyModeColorKeySmooth)
			NixSetAlpha(AlphaColorKeySmooth);
		else if (transparency_mode == TransparencyModeFunctions){
			NixSetAlpha(alpha_source, alpha_destination);
			//NixSetZ(false,false);
		}else if (transparency_mode == TransparencyModeFactor){
			NixSetAlpha(alpha_factor);
			//NixSetZ(false,false);
		}
		NixSetShader(material->shader);
	}
	if (material->cube_map >= 0){
		// evil hack
		for (int i=material->num_textures+1;i<4;i++)
			texture[i] = NULL;
		texture[3] = material->cube_map;
		NixSetTextures(texture, 4);
	}else
		NixSetTextures(texture, num_textures);
}


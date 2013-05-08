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

ModelMaterial::~ModelMaterial()
{}

void ModelMaterial::reset()
{
	// transparency
	UserTransparency = false;
	TransparencyMode = TransparencyModeDefault;
	AlphaDestination = 0;
	AlphaSource = 0;
	AlphaFactor = 50;
	AlphaZBuffer = true;

	// color
	UserColor = false;
	Ambient = White;
	Diffuse = White;
	Specular = Black;
	Emission = Black;
	Shininess = 20;

	// file
	MaterialFile = "";
	material = LoadMaterial("");

	// textures
	NumTextures = 1;
	TextureFile[0] = "";
	Texture[0] = -1;
}

void ModelMaterial::MakeConsistent()
{
	material = LoadMaterial(MaterialFile);
	CheckTextures();
	CheckTransparency();
	CheckColors();
}

void ModelMaterial::CheckTransparency()
{
	if (TransparencyMode == TransparencyModeDefault)
		UserTransparency = false;
	if (!UserTransparency){
		TransparencyMode = material->transparency_mode;
		AlphaSource = material->alpha_source;
		AlphaDestination = material->alpha_destination;
		AlphaFactor	= material->alpha_factor;
		AlphaZBuffer = material->alpha_z_buffer;
	}
}



void ModelMaterial::CheckTextures()
{
	// parent has more texture levels?
	if (material->num_textures > NumTextures){
		for (int i=NumTextures;i<material->num_textures;i++){
			TextureFile[i] = "";
			Texture[i] = -1;
		}
		NumTextures = material->num_textures;
		ed->SetMessage(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}

	// load all textures
	for (int i=0;i<NumTextures;i++)
		Texture[i] = NixLoadTexture(TextureFile[i]);

	// parent overwrites unused textures
	for (int i=0;i<material->num_textures;i++)
		if (Texture[i] < 0)
			if (TextureFile[i].num == 0)
				Texture[i] = material->texture[i];
}

void ModelMaterial::CheckColors()
{
	if (!UserColor){
		Ambient = material->ambient;
		Diffuse = material->diffuse;
		Specular = material->specular;
		Shininess = material->shininess;
		Emission = material->emission;
	}
}

void ModelMaterial::ApplyForRendering()
{
	NixSetAlpha(AlphaNone);
	NixSetShader(-1);
	color em = ColorInterpolate(Emission, White, 0.1f);
	NixSetMaterial(Ambient, Diffuse, Specular, Shininess, em);
	if (true){//MVFXEnabled){
		NixSetZ(AlphaZBuffer, AlphaZBuffer);
		if (TransparencyMode == TransparencyModeColorKeyHard)
			NixSetAlpha(AlphaColorKeyHard);
		else if (TransparencyMode == TransparencyModeColorKeySmooth)
			NixSetAlpha(AlphaColorKeySmooth);
		else if (TransparencyMode == TransparencyModeFunctions){
			NixSetAlpha(AlphaSource, AlphaDestination);
			//NixSetZ(false,false);
		}else if (TransparencyMode == TransparencyModeFactor){
			NixSetAlpha(AlphaFactor);
			//NixSetZ(false,false);
		}
		NixSetShader(material->shader);
	}
	if (material->cube_map >= 0){
		// evil hack
		Texture[3] = material->cube_map;
		NixSetTextures(Texture, 4);
	}else
		NixSetTextures(Texture, NumTextures);
}


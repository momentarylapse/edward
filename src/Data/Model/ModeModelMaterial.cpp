/*
 * ModeModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMaterial.h"
#include "../../lib/x/x.h"
#include "../../lib/nix/nix.h"
#include "../../Edward.h"


ModeModelMaterial::ModeModelMaterial()
{	reset();	}

ModeModelMaterial::~ModeModelMaterial()
{}

void ModeModelMaterial::reset()
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
	material = MetaLoadMaterial("");

	// textures
	NumTextures = 1;
	TextureFile[0] = "";
	Texture[0] = -1;
}

void ModeModelMaterial::MakeConsistent()
{
	CheckTextures();
	CheckTransparency();
	CheckColors();
}

void ModeModelMaterial::CheckTransparency()
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



void ModeModelMaterial::CheckTextures()
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

void ModeModelMaterial::CheckColors()
{
	if (!UserColor){
		Ambient = material->ambient;
		Diffuse = material->diffuse;
		Specular = material->specular;
		Shininess = material->shininess;
		Emission = material->emission;
	}
}

void ModeModelMaterial::ApplyForRendering()
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
}


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
	TransparencyMode = TransparencyModeDefault;
	AlphaDestination = 0;
	AlphaSource = 0;
	AlphaFactor = 0;
	AlphaZBuffer = true;
	Shininess = 0;
	for (int i=0;i<8;i++)
		Color[i/4][i%4] = 255; // ambient, diffuse
	for (int i=0;i<8;i++)
		Color[i/4+2][i%4] = 0; // specular, emissive
	UserColor = false;
	MaterialFile = "";
	material = MetaLoadMaterial("");
	NumTextures = 1;
	TextureFile[0] = "";
	Texture[0] = -1;
}

void ModeModelMaterial::CheckTextures()
{
	if (material->num_textures > NumTextures){
		for (int i=NumTextures;i<material->num_textures;i++){
			TextureFile[i] = "";
			Texture[i] = -1;
		}
		NumTextures = material->num_textures;
		//Change();
		ed->SetMessage(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}
	for (int i=0;i<material->num_textures;i++)
		if (Texture[i] < 0)
			if (TextureFile[i].num == 0)
				Texture[i] = material->texture[i];
}

void ModeModelMaterial::ApplyForRendering()
{
	NixSetAlpha(AlphaNone);
	NixSetShader(-1);
	color c;
	color am=material->ambient;
	color di=material->diffuse;
	color sp=material->specular;
	color em=material->emission;
	float sh=material->shininess;
	if (UserColor){
		am=i42c(Color[0]);
		di=i42c(Color[1]);
		sp=i42c(Color[2]);
		em=i42c(Color[3]);
		sh=(float)Shininess;
	}
	em=ColorInterpolate(em,White,0.1f);
	NixSetMaterial(am,di,sp,sh,em);
	if (false){//MVFXEnabled){
		int tm,as,ad;
		bool az;
		float af;
		if (TransparencyMode==TransparencyModeDefault){
			tm=material->transparency_mode;
			as=material->alpha_source;
			ad=material->alpha_destination;
			af=material->alpha_factor;
			az=material->alpha_z_buffer;
		}else{
			tm=TransparencyMode;
			as=AlphaSource;
			ad=AlphaDestination;
			af=(float)AlphaFactor*0.01f;
			az=AlphaZBuffer;
		}
		NixSetZ(az,az);
		if (tm==TransparencyModeColorKeyHard)
			NixSetAlpha(AlphaColorKeyHard);
		else if (tm==TransparencyModeColorKeySmooth)
			NixSetAlpha(AlphaColorKeySmooth);
		else if (tm==TransparencyModeFunctions){
			NixSetAlpha(as,ad);
			//NixSetZ(false,false);
		}else if (tm==TransparencyModeFactor){
			NixSetAlpha(af);
			//NixSetZ(false,false);
		}
		NixSetShader(material->shader);
	}
}


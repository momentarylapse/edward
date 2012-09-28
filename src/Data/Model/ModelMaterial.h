/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include "../../lib/x/x.h"

class ModelMaterial
{
public:
	ModelMaterial();
	virtual ~ModelMaterial();

	void reset();
	void MakeConsistent();
	void CheckTextures();
	void CheckColors();
	void CheckTransparency();
	void ApplyForRendering();

	string MaterialFile;
	Material *material;
	int NumTextures;
	int Texture[MODEL_MAX_TEXTURES];
	string TextureFile[MODEL_MAX_TEXTURES];

	bool UserTransparency;
	int TransparencyMode;
	int AlphaSource, AlphaDestination;
	float AlphaFactor;
	bool AlphaZBuffer;

	bool UserColor;
	color Ambient;
	color Diffuse;
	color Specular;
	color Emission;
	float Shininess;
};

#endif /* MODELMATERIAL_H_ */

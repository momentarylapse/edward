/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include "../../x/material.h"

class NixTexture;

class ModelMaterial
{
public:
	ModelMaterial();
	ModelMaterial(const string &filename);
	virtual ~ModelMaterial();

	void reset();
	void MakeConsistent();
	void CheckTextures();
	void CheckColors();
	void CheckTransparency();
	void ApplyForRendering();

	string material_file;
	Material *material;
	int num_textures;
	NixTexture *texture[MATERIAL_MAX_TEXTURES];
	string texture_file[MATERIAL_MAX_TEXTURES];

	bool user_transparency;
	int transparency_mode;
	int alpha_source, alpha_destination;
	float alpha_factor;
	bool alpha_zbuffer;

	bool user_color;
	color ambient;
	color diffuse;
	color specular;
	color emission;
	float shininess;
};

#endif /* MODELMATERIAL_H_ */

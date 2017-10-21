/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include "../../x/material.h"

namespace nix{
	class Texture;
	class VertexBuffer;
};

class ModelMaterial
{
public:
	ModelMaterial();
	ModelMaterial(const string &filename);
	virtual ~ModelMaterial();

	void operator=(const ModelMaterial &m);

	void reset();
	void makeConsistent();
	void checkTextures();
	void checkColors();
	void checkTransparency();
	void applyForRendering();

	string material_file;
	Material *material;
	int num_textures;
	nix::Texture *texture[MATERIAL_MAX_TEXTURES];
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

	nix::VertexBuffer *vb;
};

#endif /* MODELMATERIAL_H_ */

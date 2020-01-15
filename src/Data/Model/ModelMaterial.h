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

class Image;

class ModelMaterial
{
public:
	ModelMaterial();
	ModelMaterial(const string &filename);
	virtual ~ModelMaterial();

	void reset();
	void makeConsistent();
	void checkTextures();
	void checkColors();
	void checkTransparency();
	void applyForRendering();

	string filename;
	Material *material;

	struct TextureLevel {
		TextureLevel();
		~TextureLevel();
		string filename;
		nix::Texture *texture;
		Image *image;
		void reload_image();
		void update_texture();
	};
	Array<TextureLevel*> texture_levels;

	struct Alpha {
		bool user;
		int mode;
		int source, destination;
		float factor;
		bool zbuffer;
	} alpha;

	struct Color {
		bool user;
		color ambient;
		color diffuse;
		color specular;
		color emission;
		float shininess;
	} col;

	nix::VertexBuffer *vb;
};

#endif /* MODELMATERIAL_H_ */

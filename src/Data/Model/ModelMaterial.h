/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include "../../x/Material.h"

namespace nix {
	class Texture;
	class VertexBuffer;
};

namespace MultiView {
	class Window;
};

class Image;

class ModelMaterial {
public:
	ModelMaterial();
	ModelMaterial(const Path &filename);
	virtual ~ModelMaterial();

	void reset();
	void make_consistent();
	void check_textures();
	void check_colors();
	void check_transparency();
	void apply_for_rendering(MultiView::Window *win);

	Path filename;
	Material *material;

	struct TextureLevel {
		TextureLevel();
		~TextureLevel();
		Path filename;
		nix::Texture *texture;
		Image *image;
		bool edited;
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
		color albedo;
		float roughness;
		float metal;
		color emission;

		color ambient() const;
		color specular() const;
		float shininess() const;
		void import(const color &am, const color &di, const color &sp, float shininess, const color &em);
	} col;

	nix::VertexBuffer *vb;
};

#endif /* MODELMATERIAL_H_ */

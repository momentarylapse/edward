/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include "../../y/Material.h"
#include "../../lib/base/pointer.h"
#include "../../lib/nix/nix.h"

namespace nix {
	class Texture;
	class VertexBuffer;
	enum class Alpha;
};

namespace MultiView {
	class Window;
};

class Image;
class EdwardWindow;

class ModelMaterial {
public:
	ModelMaterial(EdwardWindow *ed, const Path &filename);
	ModelMaterial(EdwardWindow *ed);
	virtual ~ModelMaterial();
	EdwardWindow *ed;

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
		shared<nix::Texture> texture;
		Image *image;
		bool edited;
		void reload_image(EdwardWindow *ed);
		void update_texture();
	};
	Array<TextureLevel*> texture_levels;

	struct Alpha {
		bool user_defined() const;
		TransparencyMode mode;
		nix::Alpha source, destination;
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

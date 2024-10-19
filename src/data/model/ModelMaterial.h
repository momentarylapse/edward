/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include <y/world/Material.h>
#include <lib/base/pointer.h>
#include <graphics-impl.h>

namespace MultiView {
	class Window;
};

class Image;
class Session;

class ModelMaterial {
public:
	ModelMaterial(Session *s, const Path &filename);
	explicit ModelMaterial(Session *s);
	virtual ~ModelMaterial();
	Session *session;

	void reset();
	void make_consistent();
	void check_textures();
	void check_colors();
	void apply_for_rendering(MultiView::Window *win);

	Path filename;
	Material *material;
	ShaderCache shader_cache;

	struct TextureLevel {
		TextureLevel();
		~TextureLevel();
		Path filename;
		shared<Texture> texture;
		Image *image;
		bool edited;
		void reload_image(Session *s);
		void update_texture();
	};
	Array<TextureLevel> texture_levels;

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

	VertexBuffer *vb;
};

#endif /* MODELMATERIAL_H_ */

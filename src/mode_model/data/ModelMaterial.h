/*
 * ModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIAL_H_
#define MODELMATERIAL_H_

#include <lib/yrenderer/Material.h>
#include <lib/base/pointer.h>
#include <lib/image/image.h>
#include <lib/ygraphics/graphics-impl.h>

/*namespace MultiView {
	class Window;
};*/

class Session;

class ModelMaterial {
public:
	ModelMaterial(Session *s, const Path &filename);
	explicit ModelMaterial(Session *s);
	virtual ~ModelMaterial();
	Session *session;

	void make_consistent_after_shallow_loading();
	void check_textures();
	void load_colors_from_file();
//	void apply_for_rendering(MultiView::Window *win);

	Path filename;
	owned<yrenderer::Material> material;
	yrenderer::ShaderCache shader_cache;

	struct TextureLevel {
		Path filename;
		shared<ygfx::Texture> texture;
		owned<Image> image;
		bool edited = false;
		void reload_image(Session *s);
		void update_texture();
	};
	owned_array<TextureLevel> texture_levels;

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

	owned<ygfx::VertexBuffer> vb;
};

#endif /* MODELMATERIAL_H_ */

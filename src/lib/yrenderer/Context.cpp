/*
 * base.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */

#include "Context.h"
#include "lib/ygraphics/Context.h"

#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/image/image.h>

namespace yrenderer {

void Context::_create_default_textures() {
	context->_create_default_textures();
	tex_white = context->tex_white;
	tex_black = context->tex_black;
}

void Context::create_managers(const Path &texture_dir, const Path &shader_dir, const Path &material_dir) {
	texture_manager = new TextureManager(context, texture_dir);
	shader_manager = new ShaderManager(context, shader_dir);
	material_manager = new MaterialManager(this, material_dir);
}

xfer<Material> Context::load_material(const Path &filename) const {
	return material_manager->load(filename);
}

shared<ygfx::Texture> Context::load_texture(const Path& filename) const {
	return texture_manager->load_texture(filename);
}

shared<ygfx::Shader> Context::load_shader(const Path& path) const {
	return shader_manager->load_shader(path);
}
xfer<ygfx::Shader> Context::create_shader(const string &source) const {
	return shader_manager->create_shader(source);
}
shared<ygfx::Shader> Context::load_surface_shader(const Path& path, const string &render_path, const string &vertex_module, const string &geometry_module) const {
	return shader_manager->load_surface_shader(path, render_path, vertex_module, geometry_module);
}
void Context::load_shader_module(const Path& path) const {
	return shader_manager->load_shader_module(path);
}

}




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

void Context::create_managers(const Array<Path>& texture_dirs, const Array<Path>& shader_dirs, const Array<Path>& material_dirs) {
	texture_manager = new TextureManager(context, texture_dirs);
	shader_manager = new ShaderManager(context, shader_dirs);
	material_manager = new MaterialManager(texture_manager, material_dirs);
}

Material* Context::load_material(const Path &filename) const {
	return material_manager->load(filename);
}

Material *Context::create_internal_material() const {
	return material_manager->create_internal();
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
shared<ygfx::Shader> Context::load_surface_shader(const Path& path, const string &render_path, const string &vertex_module, const string &geometry_module, const string& tessellation_module) const {
	return shader_manager->load_surface_shader(path, render_path, vertex_module, geometry_module, tessellation_module);
}
void Context::load_shader_module(const Path& path) const {
	return shader_manager->load_shader_module(path);
}

}




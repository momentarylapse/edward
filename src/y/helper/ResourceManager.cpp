#include "ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/os/filesystem.h>
#include <EngineData.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/Context.h>
#include <world/ModelManager.h>
#include <world/Terrain.h>

#ifdef USING_VULKAN
namespace vulkan {
	extern string overwrite_bindings;
	extern int overwrite_push_size;
}
#endif


ResourceManager::ResourceManager(yrenderer::Context *_ctx, const Path &texture_dir, const Path &material_dir, const Path &shader_dir) {
	ctx = _ctx;
	texture_manager = new yrenderer::TextureManager(ctx ? ctx->context : nullptr, texture_dir);
	material_manager = new yrenderer::MaterialManager(texture_manager, material_dir);
	model_manager = new ModelManager(this, material_manager);
	shader_manager = new yrenderer::ShaderManager(ctx ? ctx->context : nullptr, shader_dir);
	shader_manager->ignore_missing_files = engine.ignore_missing_files;
}

xfer<yrenderer::Material> ResourceManager::load_material_copy(const Path& filename) {
	return material_manager->load_copy(filename);
}

yrenderer::Material* ResourceManager::load_material(const Path& filename) {
	return material_manager->load(filename);
}

xfer<Model> ResourceManager::load_model_copy(const Path& filename) {
	return model_manager->load_copy(filename);
}

Model* ResourceManager::load_model(const Path& filename) {
	return model_manager->load(filename);
}

Terrain *ResourceManager::load_terrain_lazy(const Path& filename) {
	if (!filename)
		return nullptr;
	for (auto t: weak(terrains))
		if (t->filename == filename)
			return t;
	auto t = new Terrain();
	t->filename = filename;
	terrains.add(t);
	return t;
}

Terrain *ResourceManager::load_terrain(const Path& filename) {
	auto t = load_terrain_lazy(filename);
	if (t and t->filename and t->height.num == 0)
		t->reload(this, true);
	return t;
}


shared<ygfx::Texture> ResourceManager::load_texture(const Path& filename) {
	return texture_manager->load_texture(filename);
}

Path ResourceManager::filename(const Model *m) const {
	return model_manager->get_filename(m);
}

Path ResourceManager::filename(const Terrain *t) const {
	for (auto _t: weak(terrains))
		if (_t == t)
			return _t->filename;
	return "";
}

Path ResourceManager::filename(const yrenderer::Material *m) const {
	return material_manager->get_filename(m);
}

Path ResourceManager::filename(const ygfx::Texture *t) const {
	return texture_manager->get_filename(t);
}

void ResourceManager::clear() {
	shader_manager->clear();
	texture_manager->clear();
	material_manager->reset();
}



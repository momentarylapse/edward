#include "ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/os/filesystem.h>
#include <y/EngineData.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/Context.h>
#include <world/ModelManager.h>

#ifdef USING_VULKAN
namespace vulkan {
	extern string overwrite_bindings;
	extern int overwrite_push_size;
}
#endif


ResourceManager::ResourceManager(yrenderer::Context *_ctx, const Path &texture_dir, const Path &material_dir, const Path &shader_dir) {
	ctx = _ctx;
	material_manager = new yrenderer::MaterialManager(ctx, material_dir);
	model_manager = new ModelManager(this, material_manager);
	shader_manager = new yrenderer::ShaderManager(ctx->context, shader_dir);
	shader_manager->ignore_missing_files = engine.ignore_missing_files;
	texture_manager = new yrenderer::TextureManager(ctx->context, texture_dir);
}

xfer<yrenderer::Material> ResourceManager::load_material(const Path &filename) {
	return material_manager->load(filename);
}

xfer<Model> ResourceManager::load_model(const Path &filename) {
	return model_manager->load(filename);
}



shared<ygfx::Texture> ResourceManager::load_texture(const Path& filename) {
	return texture_manager->load_texture(filename);
}

void ResourceManager::clear() {
	shader_manager->clear();
	texture_manager->clear();
	material_manager->reset();
}



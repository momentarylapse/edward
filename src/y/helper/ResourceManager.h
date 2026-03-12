#pragma once

#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/pointer.h>
#include <lib/os/path.h>


struct string;
class ModelManager;
class Model;
class Terrain;

namespace yrenderer {
	class Context;
	class Material;
	class MaterialManager;
	class ShaderManager;
	class TextureManager;
}

class ResourceManager {
public:
	explicit ResourceManager(yrenderer::Context *ctx, const Path &texture_dir, const Path &material_dir, const Path &shader_dir);
	yrenderer::Context* ctx;
	yrenderer::MaterialManager* material_manager;
	ModelManager* model_manager;
	yrenderer::ShaderManager* shader_manager;
	yrenderer::TextureManager* texture_manager;

	shared<ygfx::Texture> load_texture(const Path& path);
	xfer<yrenderer::Material> load_material_copy(const Path &filename);
	yrenderer::Material* load_material(const Path &filename);
	xfer<Model> load_model_copy(const Path &filename);
	Terrain* load_terrain(const Path &filename);
	Terrain* load_terrain_lazy(const Path &filename);

	owned_array<Terrain> terrains;

	void clear();
};


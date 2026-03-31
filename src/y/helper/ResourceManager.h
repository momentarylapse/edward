#pragma once

#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/pointer.h>
#include <lib/os/path.h>


struct string;
class ModelManager;
class Model;
class Terrain;
struct Template;
class TemplateManager;
class TerrainManager;

namespace yrenderer {
	class Context;
	struct Material;
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
	owned<TemplateManager> template_manager;
	owned<TerrainManager> terrain_manager;

	shared<ygfx::Texture> load_texture(const Path& path);
	yrenderer::Material* load_material(const Path &filename);
	xfer<Model> load_model_copy(const Path &filename);
	Model* load_model(const Path &filename);
	Terrain* load_terrain(const Path &filename);
	Terrain* load_terrain_lazy(const Path &filename);
	Template* load_template(const Path& filename);

	void save_template(const Template* t, const Path& filename);

	Path filename(const Model* m) const;
	Path filename(const Terrain* t) const;
	Path filename(const yrenderer::Material* m) const;
	Path filename(const ygfx::Texture* t) const;
	Path filename(const Template* t) const;

	void clear();
};


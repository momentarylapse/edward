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

namespace ygfx {
	class ShaderManager;
	class TextureManager;
}

namespace yrenderer {
	struct Context;
	struct Material;
	class MaterialManager;
}

class ResourceManager {
public:
	explicit ResourceManager(yrenderer::Context *ctx, const Path& object_dir, const Path& terrain_dir, const Array<Path>& texture_dirs, const Array<Path>& material_dirs, const Array<Path>& shader_dirs);
	void set_dirs(const Path& object_dir, const Path& terrain_dir, const Array<Path>& texture_dirs, const Array<Path>& material_dirs, const Array<Path>& shader_dirs);
	yrenderer::Context* ctx;
	yrenderer::MaterialManager* material_manager;
	ModelManager* model_manager;
	ygfx::ShaderManager* shader_manager;
	ygfx::TextureManager* texture_manager;
	owned<TemplateManager> template_manager;
	owned<TerrainManager> terrain_manager;
	Path map_dir;

	shared<ygfx::Texture> load_texture(const Path& path);
	yrenderer::Material* load_material(const Path &filename);
	yrenderer::Material* create_material();
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


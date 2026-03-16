//
// Created by michi on 8/4/25.
//

#pragma once

#include "Material.h"

namespace yrenderer {

class TextureManager;

class MaterialManager {
public:
	explicit MaterialManager(TextureManager* texture_manager, const Path& material_dir);
	~MaterialManager();

	void reset();

	void set_default(Material* m);
	Material* load(const Path& filename);
	xfer<Material> load_copy(const Path& filename);
	Path get_filename(const Material* m);

	Path material_dir;
private:
	TextureManager* texture_manager;
	Material* default_material;
	Material* trivial_material;
	base::map<Path, Material*> materials; // "originals" owned!
};

}

//
// Created by michi on 8/4/25.
//

#pragma once

#include "Material.h"

namespace yrenderer {

class MaterialManager {
public:
	explicit MaterialManager(Context *ctx, const Path& material_dir);
	~MaterialManager();

	void reset();

	void set_default(Material *m);
	void set_default_shader(ygfx::Shader *s);
	xfer<Material> load(const Path &filename);

private:
	Path material_dir;
	Context* ctx;
	ShaderManager* shader_manager;
	Material* default_material;
	Material* trivial_material;
	base::map<Path, Material*> materials; // "originals" owned!
};

}

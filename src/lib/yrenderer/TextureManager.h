//
// Created by michi on 7/30/25.
//
#pragma once

#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/pointer.h>
#include <lib/base/map.h>
#include <lib/os/path.h>

namespace yrenderer {

class TextureManager {
public:
	explicit TextureManager(ygfx::Context *ctx, const Path &texture_dir);
	ygfx::Context* ctx;

	shared<ygfx::Texture> load_texture(const Path& path);

	Path find_absolute_texture_path(const Path& path) const;

	Path texture_file(ygfx::Texture* t) const;

	Path texture_dir;
	void clear();


	shared_array<ygfx::Texture> textures;
	base::map<Path,ygfx::Texture*> texture_map;

	shared<ygfx::Texture> tex_white;

	bool ignore_missing_files = false;
};

}


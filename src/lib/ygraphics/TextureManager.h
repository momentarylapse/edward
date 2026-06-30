//
// Created by michi on 7/30/25.
//
#pragma once

#include "graphics-fwd.h"
#include <lib/base/pointer.h>
#include <lib/base/map.h>
#include <lib/os/path.h>

namespace ygfx {

class TextureManager {
public:
	explicit TextureManager(Context *ctx, const Array<Path>& texture_dirs);
	Context* ctx;

	shared<Texture> load_texture(const Path& path);

	Path find_absolute_texture_path(const Path& path) const;

	Path get_filename(const Texture* t) const;

	Array<Path> texture_dirs;
	void clear();


	shared_array<Texture> textures;
	base::map<Path,Texture*> texture_map;

	shared<Texture> tex_white;
};

}


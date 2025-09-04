//
// Created by michi on 7/30/25.
//

#include "TextureManager.h"
#include <lib/os/filesystem.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <lib/image/image.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>

namespace yrenderer {

Path guess_absolute_path(const Path &filename, const Array<Path>& dirs);

TextureManager::TextureManager(ygfx::Context *_ctx, const Path &_texture_dir) {
	ctx = _ctx;
	texture_dir = _texture_dir;

	if (ctx) {
		tex_white = ctx->tex_white;
	}
}

Path TextureManager::texture_file(ygfx::Texture* t) const {
	for (auto&& [key, _t]: texture_map)
		if (_t == t)
			return key;
	return "";
}


Path TextureManager::find_absolute_texture_path(const Path& filename) const {
	return guess_absolute_path(filename, {texture_dir});
}

shared<ygfx::Texture> TextureManager::load_texture(const Path& filename) {
	if (filename.is_empty())
		return tex_white;

	Path fn = find_absolute_texture_path(filename);
	if (fn.is_empty()) {
		if (ignore_missing_files) {
			msg_error("missing texture: " + str(filename));
			return tex_white;
		}
		throw Exception("missing texture: " + str(filename));
	}

	for (auto&& [key, t]: texture_map)
		if (fn == key) {
#ifdef USING_VULKAN
			return t;
#else
			return t->valid ? t : tex_white;
#endif
		}

	try {
#ifdef USING_VULKAN
		msg_write("loading texture: " + str(fn));
#endif
		auto t = ygfx::Texture::load(fn);
		textures.add(t);
		texture_map.add({fn, t});
		return t;
	} catch(Exception &e) {
		if (!ignore_missing_files)
			throw;
		msg_error(e.message());
		return tex_white;
	}
}

void TextureManager::clear() {
	textures.clear();
	texture_map.clear();
}

}




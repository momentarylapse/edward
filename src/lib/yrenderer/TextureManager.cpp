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

string split_filename_flags(Path& filename) {
	const string s = str(filename);
	int p = s.find("@");
	if (p < 0)
		return "";
	string flags = s.sub(p);
	filename = Path(s.sub_ref(0, p));
	return flags;
}

shared<ygfx::Texture> TextureManager::load_texture(const Path& filename_with_flags) {
	Path filename = filename_with_flags;
	string flags = split_filename_flags(filename);


	if (filename.is_empty())
		return tex_white;

	Path filename_absolute = find_absolute_texture_path(filename);
	if (filename_absolute.is_empty()) {
		msg_error("missing texture (ignore): " + str(filename));
		return tex_white;
	}
	Path filename_absolute_with_flags = filename_absolute.with(flags);

	for (auto&& [key, t]: texture_map)
		if (filename_absolute_with_flags == key)
			return t;

	msg_write("loading texture: " + str(filename_absolute_with_flags));

	auto im = ownify(Image::load(filename_absolute));
	if (!im) {
		msg_error("failed to load texture image!");
		return tex_white;
	}

	ColorSpace color_space = im->color_space;
	if (flags.find("@linear") >= 0)
		color_space = ColorSpace::Linear;

	auto t = new ygfx::Texture();
	t->write_with_color_space(*im, color_space);

	textures.add(t);
	texture_map.add({filename_absolute_with_flags, t});
	return t;
}

void TextureManager::clear() {
	textures.clear();
	texture_map.clear();
}

}




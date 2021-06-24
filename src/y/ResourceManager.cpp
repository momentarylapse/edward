#include "ResourceManager.h"
#include "../lib/nix/nix.h"
#ifdef _X_USE_HUI_
	#include "../lib/hui/hui.h"
#else
	#include "../lib/hui_minimal/Application.h"
	#include "../lib/hui_minimal/error.h"
#endif
#include "../y/EngineData.h"


Path ResourceManager::shader_dir;
Path ResourceManager::texture_dir;
static Array<nix::Shader*> shaders;
static Array<nix::Texture*> textures;

Path make_absolute_path(const Path &filename, const Array<Path> dirs) {
	if (filename.is_absolute())
		return filename;

	for (auto &d: dirs)
		if (file_exists(d << filename))
			return d << filename;

	return Path::EMPTY;
	/*if (engine.ignore_missing_files) {
		msg_error("missing shader: " + filename.str());
		return nix::Shader::load("");
	}
	throw Exception("missing shader: " + filename.str());
	return filename;*/
}

nix::Shader* ResourceManager::load_shader(const Path& filename) {
	if (filename.is_empty())
		return nix::Shader::load("");

	Path fn = make_absolute_path(filename, {shader_dir, hui::Application::directory_static << "shader"});
	if (fn.is_empty()) {
		if (engine.ignore_missing_files) {
			msg_error("missing shader: " + filename.str());
			return nix::Shader::load("");
		}
		throw Exception("missing shader: " + filename.str());
		//fn = shader_dir << filename;
	}

	for (auto s : shaders)
		if ((s->filename == fn) and (s->program >= 0))
			return s;

	auto s = nix::Shader::load(fn);
	shaders.add(s);
	return s;
}

nix::Texture* ResourceManager::load_texture(const Path& filename) {
	if (filename.is_empty())
		return nullptr;

	Path fn = make_absolute_path(filename, {texture_dir});
	if (fn.is_empty()) {
		if (engine.ignore_missing_files) {
			msg_error("missing texture: " + filename.str());
			return nullptr;
		}
		throw Exception("missing texture: " + filename.str());
	}

	for (auto t: textures)
		if (fn == t->filename)
			return t->valid ? t : nullptr;

	auto t = nix::Texture::load(fn);
	textures.add(t);
	return t;
}


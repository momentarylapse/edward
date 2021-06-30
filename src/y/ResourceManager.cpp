#include "ResourceManager.h"
#include "../lib/file/file.h"
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
Path ResourceManager::default_shader;
static Array<nix::Shader*> shaders;
static Array<nix::Texture*> textures;

Path guess_absolute_path(const Path &filename, const Array<Path> dirs) {
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

	Path fn = guess_absolute_path(filename, {shader_dir, hui::Application::directory_static << "shader"});
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
	if (!s)
		return nullptr;
	s->link_uniform_block("BoneData", 7);

	shaders.add(s);
	return s;
}

string ResourceManager::expand_shader_source(const string &source, const string &variant) {
	if (source.find("<VertexShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<VertexShader>\n#import vertex-%s\n</VertexShader>", variant);
}

nix::Shader* ResourceManager::load_surface_shader(const Path& _filename, const string &variant) {
	//nix::select_default_vertex_module("vertex-" + variant);
	//return load_shader(filename);
	auto filename = _filename;
	if (filename.is_empty())
		filename = default_shader;



	if (filename.is_empty())
		return nix::Shader::load("");

	Path fn = guess_absolute_path(filename, {shader_dir, hui::Application::directory_static << "shader"});
	if (fn.is_empty()) {
		if (engine.ignore_missing_files) {
			msg_error("missing shader: " + filename.str());
			return nix::Shader::load("");
		}
		throw Exception("missing shader: " + filename.str());
		//fn = shader_dir << filename;
	}

	Path fnx = fn.with(":" + variant);

	for (auto s : shaders)
		if ((s->filename == fnx) and (s->program >= 0))
			return s;


	msg_write("loading shader: " + fnx.str());

	string source = expand_shader_source(FileRead(fn), variant);
	auto shader = nix::Shader::create(source);
	shader->filename = fnx;

	//auto s = nix::Shader::load(fn);
	if (variant == "animated")
		if (!shader->link_uniform_block("BoneData", 7))
			msg_error("BoneData not found...");


	if (variant == "instanced")
		if (!shader->link_uniform_block("Multi", 5))
			msg_error("Multi not found...");

	shaders.add(shader);
	return shader;
}

nix::Texture* ResourceManager::load_texture(const Path& filename) {
	if (filename.is_empty())
		return nullptr;

	Path fn = guess_absolute_path(filename, {texture_dir});
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

	try {
		auto t = nix::Texture::load(fn);
		textures.add(t);
		return t;
	} catch(Exception &e) {
		if (!engine.ignore_missing_files)
			throw;
		msg_error(e.message());
		return nullptr;
	}
}


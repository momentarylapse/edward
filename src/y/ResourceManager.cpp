#include "ResourceManager.h"
#include "../lib/os/filesystem.h"
#include "../lib/os/file.h"
#include "../lib/os/msg.h"
#include "../lib/nix/nix.h"
#ifdef _X_USE_HUI_
	#include "../lib/hui/hui.h"
#else
	#include "../lib/hui_minimal/Application.h"
	#include "../lib/hui_minimal/error.h"
#endif
#include "../y/EngineData.h"
#include "../graphics-impl.h"


Path ResourceManager::shader_dir;
Path ResourceManager::texture_dir;
Path ResourceManager::default_shader;
static shared_array<Shader> shaders;
static shared_array<Texture> textures;
static base::map<Path,Shader*> shader_map;
static base::map<Path,Texture*> texture_map;

Path guess_absolute_path(const Path &filename, const Array<Path> dirs) {
	if (filename.is_absolute())
		return filename;

	for (auto &d: dirs)
		if (os::fs::exists(d | filename))
			return d | filename;

	return Path::EMPTY;
	/*if (engine.ignore_missing_files) {
		msg_error("missing shader: " + filename.str());
		return Shader::load("");
	}
	throw Exception("missing shader: " + filename.str());
	return filename;*/
}

Shader* ResourceManager::load_shader(const Path& filename) {
	if (!filename)
		return Shader::load("");

	Path fn = guess_absolute_path(filename, {shader_dir, hui::Application::directory_static | "shader"});
	if (!fn) {
		if (engine.ignore_missing_files) {
			msg_error("missing shader: " + filename.str());
			return Shader::load("");
		}
		throw Exception("missing shader: " + filename.str());
		//fn = shader_dir | filename;
	}

	for (auto s: shader_map)
		if (s.key == fn) {
#ifdef USING_VULKAN
			return s.value;
#else
			return (s.value->program >= 0) ? s.value : nullptr;
#endif
		}

#ifdef USING_VULKAN
	msg_write("loading shader: " + fn.str());
#endif
	auto s = Shader::load(fn);
	if (!s)
		return nullptr;
#ifdef USING_VULKAN
#else
	s->link_uniform_block("BoneData", 7);
#endif

	shaders.add(s);
	shader_map.add({fn, s});
	return s;
}

string ResourceManager::expand_vertex_shader_source(const string &source, const string &variant) {
	if (source.find("<VertexShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<VertexShader>\n#import vertex-%s\n</VertexShader>", variant);
}

string ResourceManager::expand_fragment_shader_source(const string &source, const string &render_path) {
	if (render_path.num > 0)
		return source.replace("#import surface", "#import surface-" + render_path);
	return source;
}

string ResourceManager::expand_geometry_shader_source(const string &source, const string &variant) {
	if (source.find("<GeometryShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<GeometryShader>\n#import geometry-%s\n</GeometryShader>", variant);
}

Shader* ResourceManager::load_surface_shader(const Path& _filename, const string &render_path, const string &variant, const string &geo) {
	msg_write("load_surface_shader: " + _filename.str() + "  " + render_path + "  " + variant + "  " + geo);
	//select_default_vertex_module("vertex-" + variant);
	//return load_shader(filename);
	auto filename = _filename;
	if (!filename)
		filename = default_shader;



	if (!filename)
		return Shader::load("");

	Path fn = guess_absolute_path(filename, {shader_dir, hui::Application::directory_static | "shader"});
	if (fn.is_empty()) {
		if (engine.ignore_missing_files) {
			msg_error("missing shader: " + filename.str());
			return Shader::load("");
		}
		throw Exception("missing shader: " + filename.str());
		//fn = shader_dir | filename;
	}

	Path fnx = fn.with(":" + variant + ":" + render_path +  ":" + geo);
	for (auto s: shader_map)
		if (s.key == fnx) {
#ifdef USING_VULKAN
			return s.value;
#else
			return (s.value->program >= 0) ? s.value : nullptr;
#endif
		}


	msg_write("loading shader: " + fnx.str());

	string source = expand_vertex_shader_source(os::fs::read_text(fn), variant);
	if (geo != "")
		source = expand_geometry_shader_source(source, geo);
	source = expand_fragment_shader_source(source, render_path);
	auto shader = Shader::create(source);

	//auto s = Shader::load(fn);
#ifdef USING_VULKAN
#else
	if (variant == "animated")
		if (!shader->link_uniform_block("BoneData", 7))
			msg_error("BoneData not found...");


	if (variant == "instanced")
		if (!shader->link_uniform_block("Multi", 5))
			msg_error("Multi not found...");
#endif


	shaders.add(shader);
	shader_map.add({filename, shader});
	return shader;
}

Shader* ResourceManager::create_shader(const string &source) {
	return Shader::create(source);
}

Texture* ResourceManager::load_texture(const Path& filename) {
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

	for (auto t: texture_map)
		if (fn == t.key) {
#ifdef USING_VULKAN
			return t.value;
#else
			return t.value->valid ? t.value : nullptr;
#endif
		}

	try {
#ifdef USING_VULKAN
		msg_write("loading texture: " + fn.str());
#endif
		auto t = Texture::load(fn);
		textures.add(t);
		texture_map.add({fn, t});
		return t;
	} catch(Exception &e) {
		if (!engine.ignore_missing_files)
			throw;
		msg_error(e.message());
		return nullptr;
	}
}

void ResourceManager::clear() {
	shaders.clear();
	shader_map.clear();
	textures.clear();
	texture_map.clear();
}


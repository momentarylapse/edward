//
// Created by michi on 7/30/25.
//

#pragma once

#include "graphics-fwd.h"
#include <lib/base/error.h>
#include <lib/base/pointer.h>
#include <lib/base/map.h>
#include <lib/os/path.h>

namespace ygfx {

class ShaderManager {
public:
	explicit ShaderManager(Context *ctx, const Array<Path>& _shader_dirs);
	void add_directory(const Path& dir);
	Context *ctx;

	base::result<shared<Shader>> load_shader(const Path& path);
	base::result<shared<Shader>> create_shader(const string &source);
	base::result<shared<Shader>> load_surface_shader(const Path& path, const string& render_path, const string& vertex_module, const string& geometry_module, const string& tessellation_module);
	static string expand_vertex_shader_source(const string &source, const string &variant);
	static string expand_fragment_shader_source(const string &source, const string &render_path);
	static string expand_geometry_shader_source(const string &source, const string &variant);
	static string expand_tessellation_control_shader_source(const string &source, const string &variant);
	static string expand_tessellation_evaluation_shader_source(const string &source, const string &variant);
	base::result_void load_shader_module(const Path& path);

	base::result<shared<Shader>> __load_shader(const Path& path, const string &overwrite_bindings, int overwrite_push_size);
	base::result<shared<Shader>> __create_shader(const string& source, const string &overwrite_bindings, int overwrite_push_size);

	Array<Path> shader_dirs;
	Path default_shader;
	void clear();


	shared_array<Shader> shaders;
	Array<Path> shader_modules;
	base::map<Path,Shader*> shader_map;
	bool ignore_missing_files = false;
};
}

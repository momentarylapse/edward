#pragma once

#include "../graphics-fwd.h"


class string;
class Path;

class ResourceManager {
public:
	static Texture *load_texture(const Path& path);
	static Shader *load_shader(const Path& path);
	static Shader *create_shader(const string &source);
	static Shader *load_surface_shader(const Path& path, const string &render_path, const string &variant);
	static string expand_vertex_shader_source(const string &source, const string &variant);
	static string expand_fragment_shader_source(const string &source, const string &render_path);
	static Path texture_dir;
	static Path shader_dir;
	static Path default_shader;
};


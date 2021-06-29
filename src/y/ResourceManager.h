#pragma once

namespace nix {
	class Texture;
	class Shader;
}

class string;
class Path;

class ResourceManager {
public:
	static nix::Texture *load_texture(const Path& path);
	static nix::Shader *load_shader(const Path& path);
	static nix::Shader *load_surface_shader(const Path& path, const string &variant);
	static string expand_shader_source(const string &source, const string &variant);
	static Path texture_dir;
	static Path shader_dir;
	static Path default_shader;
};


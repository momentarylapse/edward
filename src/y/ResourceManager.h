#pragma once

namespace nix {
	class Texture;
	class Shader;
}
class Path;

class ResourceManager {
public:
	static nix::Texture *load_texture(const Path& path);
	static nix::Shader *load_shader(const Path& path);
	static Path texture_dir;
	static Path shader_dir;
};


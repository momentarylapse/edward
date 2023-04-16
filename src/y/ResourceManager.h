#pragma once

#include "../graphics-fwd.h"
#include "../lib/base/pointer.h"


class string;
class Path;

class ResourceManager {
public:
	static shared<Texture> load_texture(const Path& path);
	static Shader *load_shader(const Path& path);
	static Shader *create_shader(const string &source);
	static Shader *load_surface_shader(const Path& path, const string &render_path, const string &variant, const string &geo);
	static string expand_vertex_shader_source(const string &source, const string &variant);
	static string expand_fragment_shader_source(const string &source, const string &render_path);
	static string expand_geometry_shader_source(const string &source, const string &variant);
	static Path texture_dir;
	static Path shader_dir;
	static Path default_shader;
	static void clear();
};


class UserMesh;
class Material;
enum class RenderPathType;

Shader *user_mesh_shader(UserMesh *m, RenderPathType type);
Shader *user_mesh_shadow_shader(UserMesh *m, Material *mat, RenderPathType type);


#pragma once

#include "../graphics-fwd.h"
#include <lib/base/pointer.h>
#include <lib/base/map.h>
#include <lib/os/path.h>


class string;
class MaterialManager;
class Material;
class ModelManager;
class Model;

class ResourceManager {
public:
	explicit ResourceManager(Context *ctx);
	Context *ctx;
	MaterialManager *material_manager;
	ModelManager *model_manager;

	shared<Texture> load_texture(const Path& path);
	Shader *load_shader(const Path& path);
	Shader *create_shader(const string &source);
	Shader *load_surface_shader(const Path& path, const string &render_path, const string &variant, const string &geo);
	string expand_vertex_shader_source(const string &source, const string &variant);
	string expand_fragment_shader_source(const string &source, const string &render_path);
	string expand_geometry_shader_source(const string &source, const string &variant);
	Material *load_material(const Path &filename);
	Model *load_model(const Path &filename);

	Shader *__load_shader(const Path& path);
	Shader *__create_shader(const string& source);

	Path texture_dir;
	Path shader_dir;
	Path default_shader;
	void clear();


	shared_array<Shader> shaders;
	shared_array<Texture> textures;
	base::map<Path,Shader*> shader_map;
	base::map<Path,Texture*> texture_map;

	shared<Texture> tex_white;
};


class UserMesh;
class Material;
enum class RenderPathType;

Shader *user_mesh_shader(ResourceManager *rm, UserMesh *m, RenderPathType type);
Shader *user_mesh_shadow_shader(ResourceManager *rm, UserMesh *m, Material *mat, RenderPathType type);


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
	shared<Shader> load_shader(const Path& path);
	xfer<Shader> create_shader(const string &source);
	shared<Shader> load_surface_shader(const Path& path, const string &render_path, const string &vertex_module, const string &geometry_module);
	string expand_vertex_shader_source(const string &source, const string &variant);
	string expand_fragment_shader_source(const string &source, const string &render_path);
	string expand_geometry_shader_source(const string &source, const string &variant);
	void load_shader_module(const Path& path);
	xfer<Material> load_material(const Path &filename);
	xfer<Model> load_model(const Path &filename);

	xfer<Shader> __load_shader(const Path& path, const string &overwrite_bindings);
	xfer<Shader> __create_shader(const string& source, const string &overwrite_bindings);

	Path texture_dir;
	Path shader_dir;
	Path default_shader;
	void clear();


	shared_array<Shader> shaders;
	Array<Path> shader_modules;
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


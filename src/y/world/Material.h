#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/pointer.h>
#include <lib/os/path.h>
#include <lib/image/color.h>
#include "../graphics-fwd.h"

#define MATERIAL_MAX_TEXTURES		8

class Path;
class Model;
class ResourceManager;


// types of transparency
enum class TransparencyMode {
	NONE = 0,
	FUNCTIONS = 1,
	COLOR_KEY_HARD = 2,
	COLOR_KEY_SMOOTH = 3,
	FACTOR = 4,
	MIX = 5,
	DEFAULT = -1,
};

// types of reflection
enum class ReflectionMode {
	NONE,
	METAL,
	MIRROR,
	CUBE_MAP_STATIC,
	CUBE_MAP_DYNAMIC
};

/*enum class ShaderVariant {
	DEFAULT,
	ANIMATED,
	INSTANCED,
	LINES,
	POINTS,
	_NUM
};*/

enum class RenderPathType;

// visual and physical properties
class Material {
public:
	ResourceManager *resource_manager;

	shared_array<Texture> textures;

	struct ShaderUniform {
		string name;
		float *p;
		int size;
	};
	Array<ShaderUniform> uniforms;
	void add_uniform(const string &name, float *p, int size);

	// light
	color albedo, emission;
	float roughness, metal;

	bool cast_shadow;

	struct RenderPassData {
		TransparencyMode mode;
		Alpha source, destination;
		float factor;
		bool z_buffer;
		int cull_mode;
		Path shader_path;
	};
	int num_passes = 1;
	RenderPassData pass0;

	struct ExtendedData {
		RenderPassData pass[4];
	};
	owned<ExtendedData> extended;

	struct Reflection {
		ReflectionMode mode;
		float density;
		//CubeMap *cube_map;
		Texture *cube_map;
		int cube_map_size;
	} reflection;

	struct Friction {
		float jump, _static, sliding, rolling;
	} friction;

	Material(ResourceManager *resource_manager);
	xfer<Material> copy();

	bool is_transparent() const;
	RenderPassData& pass(int k);
};

struct ShaderCache {
	shared<Shader> shader[2]; // * #(render paths)
	void _prepare_shader(RenderPathType render_path_type, Material *material, const string& vertex_module, const string& geometry_module);
	void _prepare_shader_multi_pass(RenderPathType render_path_type, Material *material, const string& vertex_module, const string& geometry_module, int k);
	Shader *get_shader(RenderPathType render_path_type);
};


class MaterialManager {
public:
	MaterialManager(ResourceManager *resource_manager);
	~MaterialManager();

	void reset();

	void set_default(Material *m);
	void set_default_shader(Shader *s);
	xfer<Material> load(const Path &filename);

private:
	ResourceManager *resource_manager;
	Material *default_material;
	Material *trivial_material;
	base::map<Path, Material*> materials; // "originals" owned!
};



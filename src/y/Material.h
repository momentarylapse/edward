#pragma once

#include "../lib/base/base.h"
#include "../lib/base/pointer.h"
#include "../lib/file/path.h"
#include "../lib/image/color.h"

#define MATERIAL_MAX_TEXTURES		8

class Path;
class Model;
namespace nix {
	class Texture;
	//class CubeMap;
	class Shader;
	enum class Alpha;
	enum class AlphaMode;
}



// types of transparency
enum class TransparencyMode {
	NONE = 0,
	FUNCTIONS = 1,
	COLOR_KEY_HARD = 2,
	COLOR_KEY_SMOOTH = 3,
	FACTOR = 4,
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

// visual and physical properties
class Material {
public:
	// name of the material
	Path name;

	shared_array<nix::Texture> textures;
	shared<nix::Shader> shader;

	struct ShaderUniform {
		int location;
		float *p;
		int size;
	};
	Array<ShaderUniform> uniforms;
	void add_uniform(const string &name, float *p, int size);

	// light
	color albedo, emission;
	float roughness, metal;

	bool cast_shadow;

	struct Transparency {
		TransparencyMode mode;
		nix::Alpha source, destination;
		float factor;
		bool z_buffer;
	} alpha;

	struct Reflection {
		ReflectionMode mode;
		float density;
		//CubeMap *cube_map;
		nix::Texture *cube_map;
		int cube_map_size;
	} reflection;

	struct Friction {
		float jump, _static, sliding, rolling;
	} friction;

	Material();
	~Material();
	Material *copy();
};




// management
void MaterialInit();
void MaterialEnd();
void MaterialReset();
void SetDefaultMaterial(Material *m);
void MaterialSetDefaultShader(nix::Shader *s);
Material *LoadMaterial(const Path &filename);


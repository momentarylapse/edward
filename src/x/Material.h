#ifndef SRC_WORLD_MATERIAL_H_
#define SRC_WORLD_MATERIAL_H_

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
}


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
		int mode;
		int source, destination;
		float factor;
		bool z_buffer;
	} alpha;

	struct Reflection {
		int mode;
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


// types of transparency
enum {
	TRANSPARENCY_NONE,
	TRANSPARENCY_FUNCTIONS,
	TRANSPARENCY_COLOR_KEY_HARD,
	TRANSPARENCY_COLOR_KEY_SMOOTH,
	TRANSPARENCY_FACTOR,
	TRANSPARENCY_DEFAULT = -1,
};

// types of reflection
enum {
	REFLECTION_NONE,
	REFLECTION_METAL,
	REFLECTION_MIRROR,
	REFLECTION_CUBE_MAP_STATIC,
	REFLECTION_CUBE_MAP_DYNAMIC
};



// management
extern Path MaterialDir;
void MaterialInit();
void MaterialEnd();
void MaterialReset();
void SetDefaultMaterial(Material *m);
void MaterialSetDefaultShader(nix::Shader *s);
Material *LoadMaterial(const Path &filename);

#endif

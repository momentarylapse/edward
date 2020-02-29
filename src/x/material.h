#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "../lib/base/base.h"
#include "../lib/image/color.h"

#define MATERIAL_MAX_TEXTURES		8

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
	string name;

	Array<nix::Texture*> textures;
	nix::Shader *shader;

	// light
	color ambient, diffuse, specular, emission;
	float shininess;

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
extern string MaterialDir;
void MaterialInit();
void MaterialEnd();
void MaterialReset();
void SetDefaultMaterial(Material *m);
Material *LoadMaterial(const string &filename);

#endif

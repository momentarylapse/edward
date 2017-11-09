#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "../lib/base/base.h"
#include "../lib/image/color.h"

#define MATERIAL_MAX_TEXTURES		8

class Model;
namespace nix{
	class Texture;
	class CubeMap;
	class Shader;
};


// visual and physical properties
class Material
{
public:
	// name of the material
	string name;

	// textures
	Array<nix::Texture*> textures;

	// light
	color ambient, diffuse, specular, emission;
	float shininess;

	// transparency
	int transparency_mode;
	int alpha_source, alpha_destination;
	float alpha_factor;
	bool alpha_z_buffer;

	// reflection
	int reflection_mode;
	float reflection_density;
	nix::CubeMap *cube_map;
	int cube_map_size;

	// shader
	nix::Shader *shader;

	// friction
	float rc_jump, rc_static, rc_sliding, rc_rolling;

	Material();
	~Material();
	void apply();
	void copy_from(Model *model, Material *m, bool user_colors);
};


// types of transparency
enum{
	TRANSPARENCY_NONE,
	TRANSPARENCY_FUNCTIONS,
	TRANSPARENCY_COLOR_KEY_HARD,
	TRANSPARENCY_COLOR_KEY_SMOOTH,
	TRANSPARENCY_FACTOR,
	TRANSPARENCY_DEFAULT = -1,
};

// types of reflection
enum
{
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
Material *LoadMaterial(const string &filename, bool as_default = false);

#endif

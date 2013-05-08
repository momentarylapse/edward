#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "../lib/base/base.h"
#include "../lib/image/color.h"

#define MATERIAL_MAX_TEXTURES		8

class Model;


// visual and physical properties
struct Material
{
	// name of the material
	string name;

	// textures
	int num_textures;
	int texture[MATERIAL_MAX_TEXTURES];

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
	int cube_map, cube_map_size;

	// shader
	int shader;

	// friction
	float rc_jump, rc_static, rc_sliding, rc_rolling;

	Material();
	~Material();
	void apply();
	void copy_from(Model *model, Material *m, bool user_colors);
};


// types of transparency
#define TransparencyModeDefault			-1
#define TransparencyModeNone			0
#define TransparencyModeFunctions		1
#define TransparencyModeColorKeyHard	2
#define TransparencyModeColorKeySmooth	3
#define TransparencyModeFactor			4


// types of reflection
enum
{
	ReflectionNone,
	ReflectionMetal,
	ReflectionMirror,
	ReflectionCubeMapStatic,
	ReflectionCubeMapDynamical
};



// management
extern string MaterialDir;
void MaterialInit();
void MaterialEnd();
void MaterialReset();
Material *LoadMaterial(const string &filename, bool as_default = false);

#endif

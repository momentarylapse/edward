/*
 * LevelData.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#ifndef SRC_WORLD_LEVELDATA_H_
#define SRC_WORLD_LEVELDATA_H_


#include <lib/base/base.h>
#include <lib/os/path.h>
#include <lib/math/vec3.h>
#include <lib/image/color.h>
#include <y/Entity.h>

struct LevelData;


class Model;
class Object;
namespace yrenderer {
	class Material;
	enum class LightType;
}
class Terrain;
struct ScriptInstanceData;
class Light;
class ParticleManager;
class Particle;


enum class PhysicsMode;
enum class LinkType;

struct Fog {
	bool enabled;
	int mode;
	float start, end, distance;
	color _color;
};


struct LevelData {
	LevelData();
	bool load(const Path &filename);
	void save(const Path &filename);

	struct Template {
		Array<ScriptInstanceData> components;
	};
	static Template load_template(const Path &filename);
    static void save_template(const Template& t, const Path &filename);
	static Array<ScriptInstanceData> auto_terrain_components();


	struct Terrain {
		Path filename;
		vec3 pos;
		Array<ScriptInstanceData> components;
	};

	struct Object {
		Path filename;
		string name;
		vec3 pos, ang;
		Array<ScriptInstanceData> components;
	};

	struct Entity {
		string name;
		vec3 pos;
		quaternion ang;
		Array<ScriptInstanceData> components;
	};

	struct Light {
		bool enabled;
		vec3 pos, ang;
		yrenderer::LightType type;
		color _color; // physical units
		float radius, theta, harshness;
		Array<ScriptInstanceData> components;
	};

	struct Camera {
		vec3 pos, ang;
		float fov, min_depth, max_depth, exposure, bloom_factor;
		Array<ScriptInstanceData> components;
	};

	struct Link {
		int object[2];
		LinkType type;
		vec3 pos, ang;
		Array<ScriptInstanceData> components;
	};

	Path world_filename;
	Array<Path> skybox_filename;
	Array<vec3> skybox_ang;
	color background_color;
	Array<Object> objects;
	Array<Terrain> terrains;
	Array<Entity> entities;
	int ego_index;
	Array<ScriptInstanceData> systems;
	Array<Light> lights;
	Array<Link> links;

	Array<Camera> cameras;

	bool physics_enabled;
	PhysicsMode physics_mode;
	vec3 gravity;
	Fog fog;
};



#endif /* SRC_WORLD_LEVELDATA_H_ */

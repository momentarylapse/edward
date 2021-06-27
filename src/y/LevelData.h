/*
 * LevelData.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#ifndef SRC_WORLD_LEVELDATA_H_
#define SRC_WORLD_LEVELDATA_H_


#include "../lib/base/base.h"
#include "../lib/file/path.h"
#include "../lib/math/vector.h"
#include "../lib/image/color.h"
#include "../y/Entity.h"

class LevelData;


class Model;
class Object;
class Material;
class Terrain;
class TemplateDataScriptVariable;
class Light;
class ParticleManager;
class Particle;


enum class PhysicsMode;
enum class LinkType;

class Fog {
public:
	bool enabled;
	int mode;
	float start, end, distance;
	color _color;
};


class LevelData {
public:
	LevelData();
	bool load(const Path &filename);


	class Terrain {
	public:
		Path filename;
		vector pos;
	};

	class Object {
	public:
		Path filename;
		Path script;
		string name;
		vector pos, ang, vel, rot;
	};

	class Light {
	public:
		bool enabled;
		vector pos, ang;
		color _color;
		float radius, theta, harshness;
	};

	class Camera {
	public:
		vector pos, ang;
		float fov, min_depth, max_depth, exposure;
	};

	class Script {
	public:
		Path filename;
		Array<TemplateDataScriptVariable> variables;
	};

	class Link {
	public:
		int object[2];
		LinkType type;
		vector pos, ang;
	};

	Path world_filename;
	Array<Path> skybox_filename;
	Array<vector> skybox_ang;
	color background_color;
	Array<Object> objects;
	Array<Terrain> terrains;
	int ego_index;
	Array<Script> scripts;
	Array<Light> lights;
	Array<Link> links;

	Array<Camera> cameras;

	bool physics_enabled;
	PhysicsMode physics_mode;
	vector gravity;
	Fog fog;
};



#endif /* SRC_WORLD_LEVELDATA_H_ */

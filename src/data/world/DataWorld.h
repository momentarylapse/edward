/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include "../Data.h"
#include "../../multiview/SingleData.h"
#include "../../y/Material.h"

class DataWorld;
class Terrain;
class Object;
class WorldLink;
class WorldLight;
class WorldObject;
class WorldTerrain;
class WorldCamera;
enum class PhysicsMode;

namespace nix {
	enum class FogMode;
}


class WorldScriptVariable {
public:
	string name;
	string type;
	string value;
};

class ScriptInstanceData {
public:
	Path filename;
	string class_name;
	Array<WorldScriptVariable> variables;
};

class WorldScript : public ScriptInstanceData {};

class DataWorld: public Data {
public:
	DataWorld(Session *ed);
	virtual ~DataWorld();

	void reset() override;


	void get_bounding_box(vec3 &min, vec3 &max);
	int get_selected_objects();
	int get_selected_terrains();
	int get_selected_cameras();
	int get_selected_lights();

	void update_data();

	void clear_selection();

	// terrains
	Array<WorldTerrain> terrains;

	// objects
	Array<WorldObject> objects;
	int EgoIndex;

	Array<WorldLink> links;

	Array<WorldLight> lights;
	Array<WorldCamera> cameras;

	struct MetaData {

		// physics
		bool physics_enabled;
		PhysicsMode physics_mode;
		vec3 gravity;

		// background
		color background_color;
		Array<Path> skybox_files;

		// fog
		struct Fog {
			bool enabled;
			nix::FogMode mode;
			float start;
			float end;
			float density;
			color col;
		} fog;

		// scripts
		Array<WorldScript> scripts;

		// music
		Array<Path> music_files;

		void reset();
	};
	MetaData meta_data;



	// actions
	WorldObject *add_object(const Path &filename, const vec3 &pos);
	WorldTerrain *add_terrain(const Path &filename, const vec3 &pos);
	WorldTerrain *add_new_terrain(const vec3 &pos, const vec3 &size, int num_x, int num_z);
	WorldCamera *add_camera(const WorldCamera& c);
	void edit_camera(int index, const WorldCamera& c);
	WorldLight *add_light(const WorldLight& l);
	void edit_light(int index, const WorldLight& l);

	void copy(Array<WorldObject> &objects, Array<WorldTerrain> &terrains, Array<WorldCamera> &cameras, Array<WorldLight> &lights); // actually not an action
	void paste(Array<WorldObject> &objects, Array<WorldTerrain> &terrains, Array<WorldCamera> &cameras, Array<WorldLight> &lights);
	void delete_selection();
};

#endif /* DATAWORLD_H_ */

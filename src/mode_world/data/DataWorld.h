/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include <Session.h>
#include "WorldCamera.h"
#include "WorldLight.h"
#include "WorldObject.h"
#include "WorldTerrain.h"
#include "../../data/Data.h"
#include "../../multiview/SingleData.h"
#include <y/world/Material.h>
#include <y/graphics-fwd.h>
#include <lib/any/any.h>
#include <lib/math/quaternion.h>

class DataWorld;
class Terrain;
class Object;
struct WorldLink;
struct WorldLight;
struct WorldObject;
struct WorldTerrain;
struct WorldCamera;
enum class PhysicsMode;
class Entity;
class Component;


struct WorldScriptVariable {
	string name;
	string type;
	string value;
};

struct ScriptInstanceData {
	Path filename;
	string class_name;
	Array<WorldScriptVariable> variables;
};

struct WorldComponent {
	Path filename;
	string class_name;
	Any data;
	Component* component = nullptr;
};

struct WorldEntity : multiview::SingleData {
	quaternion ang = quaternion::ID;

	MultiViewType basic_type = MultiViewType::WORLD_ENTITY;
	WorldLight light;
	WorldCamera camera;
	WorldObject object;
	WorldTerrain terrain;

	//Entity* entity = nullptr;
	Array<ScriptInstanceData> components;
};

class DataWorld: public Data {
public:
	explicit DataWorld(Session *ed);
	~DataWorld() override;

	void reset() override;
	void add_initial_data();
	bool is_empty() const;


	void get_bounding_box(vec3 &min, vec3 &max);

	void update_data();

	void clear_selection();

	Array<WorldEntity> entities;
	int EgoIndex;

	Array<WorldLink> links;

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
			FogMode mode;
			float start;
			float end;
			float density;
			color col;
		} fog;

		// scripts
		Array<ScriptInstanceData> systems;

		// music
		Array<Path> music_files;

		void reset();
	};
	MetaData meta_data;


	Selection get_selection() const override;

#if 0
	// actions
	WorldObject *add_object(const Path &filename, const vec3 &pos);
	WorldTerrain *add_terrain(const Path &filename, const vec3 &pos);
	WorldTerrain *add_new_terrain(const vec3 &pos, const vec3 &size, int num_x, int num_z);
	WorldCamera *add_camera(const WorldCamera& c);
#endif
	WorldEntity* add_entity(const WorldEntity& e);
	void edit_camera(int index, const WorldCamera& c);
	void edit_light(int index, const WorldLight& l);
	void entity_add_component(int index, const ScriptInstanceData& c);
	void entity_remove_component(int index, int cindex);
	void entity_edit_component(int index, int cindex, const ScriptInstanceData& c);

	void copy(DataWorld& temp) const; // actually not an action
	void paste(const DataWorld& temp);
	void delete_selection(const Data::Selection& selection);
};

#endif /* DATAWORLD_H_ */

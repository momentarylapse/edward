/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include <Session.h>
#include "WorldObject.h"
#include "WorldTerrain.h"
#include <data/Data.h>
#include <view/SingleData.h>
#include <lib/yrenderer/Material.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/any/any.h>
#include <lib/math/quaternion.h>

#include "fx/ParticleEmitter.h"
//#include <y/EntityManager.h>

class DataWorld;
class Terrain;
class Object;
struct WorldLink;
struct WorldObject;
struct WorldTerrain;
enum class PhysicsMode;
class Entity;
class Component;
class EntityManager;


struct WorldScriptVariable {
	string name;
	string type;
	string value;
};

struct ScriptInstanceData {
	Path filename;
	string class_name;
	Array<WorldScriptVariable> variables;
	string get(const string& name) const;
	void set(const string& name, const string& type, const string& value);
};

/*struct WorldComponent {
	Path filename;
	string class_name;
	Any data;
	Component* component = nullptr;
};*/

struct WorldEntity { //: multiview::SingleData {
	vec3 pos = vec3::ZERO;
	quaternion ang = quaternion::ID;
	int view_stage = -1;

	MultiViewType basic_type = MultiViewType::WORLD_ENTITY;
	WorldObject object;
	WorldTerrain terrain;

	//Entity* entity = nullptr;
	Array<ScriptInstanceData> components;

	ScriptInstanceData& get(const string& class_name);
};

struct EdwardTag : Component {
	int entity_index;
	Array<ScriptInstanceData> user_components;
	ScriptInstanceData& get(const string& class_name);

	static const kaba::Class* _class;
};

class DataWorld: public Data {
public:
	explicit DataWorld(Session *ed);
	~DataWorld() override;

	obs::source out_component_added{this, "component-added"};
	obs::source out_component_removed{this, "component-removed"};

	void reset() override;
	void add_initial_data();
	bool is_empty() const;


	Box get_bounding_box() const;

	void update_data();

	Array<WorldEntity> entities;
	int EgoIndex;

	owned<EntityManager> entity_manager;
	Array<multiview::SingleData> dummy_entities;
	Entity* entity(int index);

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
			ygfx::FogMode mode;
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


	//Selection get_selection() const override;

#if 0
	// actions
	WorldObject *add_object(const Path &filename, const vec3 &pos);
	WorldTerrain *add_terrain(const Path &filename, const vec3 &pos);
	WorldTerrain *add_new_terrain(const vec3 &pos, const vec3 &size, int num_x, int num_z);
	WorldCamera *add_camera(const WorldCamera& c);
#endif
	Entity* add_entity(const vec3& pos, const quaternion& ang);
	void edit_entity(int index, const vec3& pos, const quaternion& ang);
	void edit_terrain_meta_data(int index, const vec3& pattern);
	Component* entity_add_component_generic(int index, const kaba::Class* type, const base::map<string, Any>& variables = {});//, const ScriptInstanceData& c);
	template<class T>
	T* entity_add_component(int index, const base::map<string, Any>& variables = {}) {
		return static_cast<T*>(entity_add_component_generic(index, T::_class, variables));
	}
	void entity_remove_component(int index, const kaba::Class* type);
	void entity_edit_component(int index, const kaba::Class* type, const ScriptInstanceData& c);
	void entity_add_user_component(int index, const ScriptInstanceData& c);
	void entity_remove_user_component(int index, int cindex);
	void entity_edit_user_component(int index, int cindex, const ScriptInstanceData& c);

	void copy(DataWorld& temp, const Data::Selection& sel) const; // actually not an action
	void paste(const DataWorld& temp);
	void delete_selection(const Data::Selection& selection);
};

#endif /* DATAWORLD_H_ */

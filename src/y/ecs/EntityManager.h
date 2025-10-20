//
// Created by michi on 9/10/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/pattern/Observable.h>
#include "ComponentManager.h"

struct vec3;
struct quaternion;
class Entity;


struct EntityMessageParams {
	Entity* entity;
	Component* component;
};

class EntityManager : public obs::Node<VirtualBase> {
public:
	EntityManager();
	~EntityManager() override;

	obs::xsource<EntityMessageParams> out_add_component{this, "add-component"};
	obs::xsource<EntityMessageParams> out_remove_component{this, "remove-component"};

	using Params = ComponentManager::Params;

	Entity* create_entity(const vec3& pos, const quaternion& ang);
	void delete_entity(Entity* entity);
	int entity_index(Entity* entity) const;

	Component *_add_component_generic_(Entity* entity, const kaba::Class *type, const Params &var = {});
	void _add_component_external_(Entity* entity, Component *c);
	void delete_component(Entity* entity, Component *c, bool notify=true);

	template<class C>
	C* add_component(Entity* entity, const Params& var = {}) {
		return static_cast<C*>(_add_component_generic_(entity, C::_class, var));
	}

	void reset();
	void shift_all(const vec3& dpos);

	template<class C>
	Array<C*>& get_component_list() {
		return (Array<C*>&) component_manager->_get_list(C::_class);
	}

	template<class C>
	Array<const C*>& get_component_list_const() const {
		return (Array<const C*>&) const_cast<ComponentManager*>(component_manager.get())->_get_list(C::_class);
	}

	template<class C>
	Array<C*>& get_component_list_family() {
		return (Array<C*>&) component_manager->_get_list_family(C::_class);
	}

	static EntityManager* global;
	owned<ComponentManager> component_manager;

//private:
	Array<Entity*> entities;
	bool init_components = true;
};


//
// Created by michi on 9/10/25.
//

#include "EntityManager.h"
#include "Entity.h"
#include "Component.h"

EntityManager* EntityManager::global = nullptr;

EntityManager::EntityManager() {
	global = this;
	component_manager = new ComponentManager();
}

EntityManager::~EntityManager() {
	reset();
}

Entity *EntityManager::create_entity(const vec3 &pos, const quaternion &ang) {
	auto e = new Entity(pos, ang);
	entities.add(e);
	return e;
}

void EntityManager::delete_entity(Entity* e) {
	int index = entities.find(e);
	if (index < 0)
		return;

	while (e->components.num > 0)
		delete_component(e, e->components.back());

	//e->on_delete_rec();

	entities.erase(index);
	delete e;
}


Component *EntityManager::_add_component_generic_(Entity* entity, const kaba::Class *type, const Params &var) {
	auto c = component_manager->create_component(type, var);
	entity->components.add(c);
	c->owner = entity;
	c->on_init();
	return c;
}

void EntityManager::_add_component_external_(Entity* entity, Component *c) {
	component_manager->_register(c);
	entity->components.add(c);
	c->owner = entity;
	c->on_init();
}

void EntityManager::delete_component(Entity* entity, Component *c, bool notify) {
	int i = entity->components.find(c);
	if (i >= 0) {
		if (notify)
			c->on_delete();
		c->owner = nullptr;
		entity->components.erase(i);
		component_manager->delete_component(c);
	}
}


void EntityManager::shift_all(const vec3 &dpos) {
	for (auto *e: entities)
		e->pos += dpos;
}


void EntityManager::reset() {
	for (auto e: entities) {
		while (e->components.num > 0)
			delete_component(e, e->components.back(), false);

		delete e;
	}
	entities.clear();
}




/*
 * Entity.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "Entity.h"
#include "Component.h"
#include "ComponentManager.h"
#include "../lib/kaba/syntax/Class.h"


Array<Entity*> EntityManager::selection;



Entity::Entity(Type t) {
	type = t;
}

// hmm, no, let's not do too much here...
//   one might expect to call on_delete() here, but that's not possible,
//   since all outer destructors have been called at this point already
Entity::~Entity() {
	//msg_write("~Entity " + i2s((int)type));
	for (auto *c: components)
		ComponentManager::delete_component(c);
/*	//msg_write("~Entity " + i2s((int)type));
	if (EntityManager::enabled) {
		//msg_write("auto unreg...");
		world.unregister(this);
	}
	//msg_write("/~Entity " + i2s((int)type));*/
}

void Entity::on_init_rec() {
	on_init();
	for (auto c: components)
		c->on_init();
}

void Entity::on_delete_rec() {
	for (auto c: components)
		c->on_delete();
	on_delete();
}


// TODO (later) optimize...
Component *Entity::add_component(const kaba::Class *type, const string &var) {
	auto c = add_component_no_init(type, var);
//	c->on_init();
	return c;
}

Component *Entity::add_component_no_init(const kaba::Class *type, const string &var) {
	auto c = ComponentManager::create_component(type, var);
	components.add(c);
	c->owner = this;
	return c;
}

void Entity::_add_component_external_(Component *c) {
	ComponentManager::add_to_list(c, ComponentManager::get_component_type_family(c->component_type));
	components.add(c);
	c->owner = this;
	//c->on_init();
}

Component *Entity::_get_component_untyped_(const kaba::Class *type) const {
	for (auto *c: components)
		if (c->component_type->is_derived_from(type))
			return c;
	return nullptr;
}



void EntityManager::reset() {
	selection.clear();
}


void EntityManager::delete_later(Entity *p) {
	selection.add(p);
}

void EntityManager::delete_selection() {
	for (auto *p: selection)
		delete p;
	selection.clear();
}


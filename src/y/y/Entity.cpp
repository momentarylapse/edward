/*
 * Entity.cpp
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#include "Entity.h"
#include "Component.h"
#include "ComponentManager.h"
#include <lib/math/mat4.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/os/msg.h>


Entity::Entity() : Entity(vec3::ZERO, quaternion::ID) {}

Entity::Entity(const vec3 &_pos, const quaternion &_ang) : BaseClass(BaseClass::Type::ENTITY) {
	pos = _pos;
	ang = _ang;
	parent = nullptr;
	object_id = -1;
}

// hmm, no, let's not do too much here...
//   one might expect to call on_delete() here, but that's not possible,
//   since all outer destructors have been called at this point already
Entity::~Entity() {
	for (auto *c: components) {
		c->owner = nullptr;
		ComponentManager::delete_component(c);
	}
}

void Entity::on_init_rec() {
	//msg_write("init rec");
	on_init();
	for (auto c: components) {
		//msg_write(" -> " + c->component_type->name);
		c->on_init();
	}
}

void Entity::on_delete_rec() {
	for (auto c: components)
		c->on_delete();
	on_delete();
}


// TODO (later) optimize...
Component *Entity::_add_component_untyped_(const kaba::Class *type, const string &var) {
	auto c = add_component_no_init(type, var);

	c->on_init();
	return c;
}

Component *Entity::add_component_no_init(const kaba::Class *type, const string &var) {
	auto c = ComponentManager::create_component(type, var);
	components.add(c);
	c->owner = this;
	return c;
}

void Entity::_add_component_external_no_init_(Component *c) {
	ComponentManager::_register(c);
	components.add(c);
	c->owner = this;
}

void Entity::delete_component(Component *c) {
	int i = components.find(c);
	if (i >= 0) {
		c->on_delete();
		c->owner = nullptr;
		components.erase(i);
		ComponentManager::delete_component(c);
	}
}

Component *Entity::_get_component_untyped_(const kaba::Class *type) const {
	//msg_write("get " + type->name);
	for (auto *c: components) {
		//msg_write(p2s(c->component_type));
		//msg_write("... " + c->component_type->name);
		if (c->component_type->is_derived_from(type))
			return c;
	}
	return nullptr;
}


mat4 Entity::get_local_matrix() const {
	return mat4::translation(pos) * mat4::rotation(ang);
}

mat4 Entity::get_matrix() const {
	if (parent)
		return parent->get_matrix() * get_local_matrix();
	return get_local_matrix();
}


Entity *Entity::root() const {
	Entity *next = const_cast<Entity*>(this);
	while (next->parent)
		next = next->parent;
	return next;
}


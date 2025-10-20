/*
 * Entity.cpp
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#include "Entity.h"
#include "Component.h"
#include <lib/math/mat4.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/os/msg.h>


Entity::Entity() : Entity(vec3::ZERO, quaternion::ID) {}

Entity::Entity(const vec3 &_pos, const quaternion &_ang) {
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
		msg_error(format("deleting entity %s while component %s still attached", p2s(this), c->component_type->long_name()));
		int*p = nullptr;
		*p = 13;
	}
}

void Entity::on_delete_rec() {
	for (auto c: components)
		c->on_delete();
}

Component *Entity::_get_component_derived_generic_(const kaba::Class *type) const {
	for (auto *c: components)
		if (c->component_type->is_derived_from(type))
			return c;
	return nullptr;
}

Component *Entity::_get_component_generic_(const kaba::Class *type) const {
	for (auto *c: components)
		if (c->component_type == type)
			return c;
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


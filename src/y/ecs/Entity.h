/*
 * Entity.h
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#pragma once

#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include "BaseClass.h"

struct mat4;
class Component;
class EntityManager;


class Entity {
public:
	Entity();
	Entity(const vec3 &pos, const quaternion &ang);
	~Entity();

	void on_delete_rec();

	Array<Component*> components;
	Component *_get_component_generic_(const kaba::Class *type) const;
	Component *_get_component_derived_generic_(const kaba::Class *type) const;

	template<class C>
	C* get_component() const {
		return static_cast<C*>(_get_component_generic_(C::_class));
	}

	template<class C>
	C* get_component_derived() const {
		return static_cast<C*>(_get_component_derived_generic_(C::_class));
	}


	vec3 pos;
	quaternion ang;
	mat4 get_local_matrix() const;
	mat4 get_matrix() const;

	int object_id;
	Entity *parent;
	Entity *_cdecl root() const;
};


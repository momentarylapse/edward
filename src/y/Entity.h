/*
 * Entity.h
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#pragma once

#include "../lib/math/vec3.h"
#include "../lib/math/quaternion.h"
#include "BaseClass.h"

class mat4;
class Component;


class Entity : public BaseClass {
public:
	Entity();
	Entity(const vec3 &pos, const quaternion &ang);
	~Entity();

	void on_init_rec();
	void on_delete_rec();

	Array<Component*> components;
	Component *_get_component_untyped_(const kaba::Class *type) const;
	Component *add_component(const kaba::Class *type, const string &var);
	Component *add_component_no_init(const kaba::Class *type, const string &var);

	template<class C>
	C *get_component() const {
		return (C*)_get_component_untyped_(C::_class);
	}

	void _add_component_external_(Component *c);


	vec3 pos;
	quaternion ang;
	mat4 get_local_matrix() const;
	mat4 get_matrix() const;

	int object_id;
	Entity *parent;
	Entity *_cdecl root() const;
};


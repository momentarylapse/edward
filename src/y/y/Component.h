/*
 * Component.h
 *
 *  Created on: Jul 12, 2021
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

class Entity;
class CollisionData;
namespace kaba {
	class Class;
}

class Component : public VirtualBase {
public:
	Component();
	~Component() override;
	virtual void on_init() {}
	virtual void on_delete() {}
	virtual void on_iterate(float dt) {}

	virtual void on_collide(const CollisionData &col) {}

	void set_variables(const string &var);

	Entity *owner;
	const kaba::Class *component_type;

	/*template<class Owner>
	Owner *get_owner() const { return (Owner*)owner; };*/
};

class NameTag : public Component {
public:
	string name;
	static const kaba::Class* _class;
};


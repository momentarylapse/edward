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
struct ScriptInstanceDataVariable;
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

	virtual void on_collide(const CollisionData& col) {}

	void set_variables(const Array<ScriptInstanceDataVariable>& variables);

	Entity* owner;
	const kaba::Class* component_type;
};

class NameTag : public Component {
public:
	string name;
	static const kaba::Class* _class;
};

class EgoMarker : public Component {
public:
	static const kaba::Class* _class;
};


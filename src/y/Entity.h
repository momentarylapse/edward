/*
 * Entity.h
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#ifndef SRC_Y_ENTITY_H_
#define SRC_Y_ENTITY_H_

#include "../lib/base/base.h"


class Entity : public VirtualBase {
public:
	enum class Type {
		NONE,
		CONTROLLER,
		MODEL,
		TERRAIN,
		CAMERA,
		LIGHT,
		LINK,
		SOUND,
		EFFECT,
		PARTICLE,
		BEAM,
		UI_NODE,
		UI_TEXT,
		UI_PICTURE,
		UI_MODEL,
	};

	Entity(Type t);
	virtual ~Entity();
	virtual void _cdecl on_iterate(float dt){}
	virtual void _cdecl on_init(){}
	virtual void _cdecl on_delete(){}

	Type type;
};


class EntityManager {
public:
	static void reset();
	static void delete_later(Entity *p);
	static void delete_selection();
	static bool enabled;
private:
	static Array<Entity*> selection;
};

#define entity_reg(var, array) \
	if (EntityManager::enabled) \
		(array).add(var);

#define entity_unreg(var, array) \
	if (EntityManager::enabled) \
		for (int i=0;i<(array).num;i++) \
			if ((array)[i] == var) \
				(array).erase(i);

#define entity_del(array) \
	for (int i=(array).num-1; i>=0; i--) \
		delete((array)[i]); \
	(array).clear();

#endif /* SRC_Y_ENTITY_H_ */

/*
 * ComponentManager.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "ComponentManager.h"
#include "Component.h"
#include "Entity.h"
#include <lib/base/map.h>
#include <lib/config.h>
#include <lib/profiler/Profiler.h>
#include <lib/kaba/syntax/Class.h>
#include <lib/kaba/syntax/Function.h>
#include <lib/os/msg.h>

class ComponentListX {
public:
	ComponentManager::List list;
	bool needs_update = false;
	const kaba::Class *type_family = nullptr;
	int ch_iterate = -1;

	void add(Component *c) {
		list.add(c);
	}
	void remove(Component *c) {
		foreachi (auto *cc, list, i)
			if (cc == c) {
				list.erase(i);
				return;
			}
		msg_error("failed to remove component from list: " + c->component_type->name);
	}
};

bool class_func_did_override(const kaba::Class *type, const string &fname) {
	for (auto f: weak(type->functions))
		if (f->name == fname)
			return f->name_space != type->get_root();
	return false;
}

ComponentListX &_get_list_x(ComponentManager* cm, const kaba::Class *type) {
	if (cm->component_lists_by_type.find(type) >= 0) {
		return cm->component_lists_by_type[type];
	} else {
		ComponentListX list;
		list.type_family = type;
		list.needs_update = class_func_did_override(type, "on_iterate");
		if (list.needs_update)
			list.ch_iterate = profiler::create_channel(type->long_name(), cm->ch_component);
		cm->component_lists_by_type.set(type, list);
		return cm->component_lists_by_type[type];
	}
}

ComponentListX &_get_list_x_family(ComponentManager* cm, const kaba::Class *type_family) {
	if (cm->component_lists_by_family.find(type_family) >= 0) {
		return cm->component_lists_by_family[type_family];
	} else {
		ComponentListX list;
		list.type_family = type_family;
		list.needs_update = class_func_did_override(type_family, "on_iterate");
		if (list.needs_update)
			list.ch_iterate = profiler::create_channel(type_family->long_name(), cm->ch_component);
		cm->component_lists_by_family.set(type_family, list);
		return cm->component_lists_by_family[type_family];
	}
}

ComponentManager::ComponentManager() {
	ch_component = profiler::create_channel("component");
}

ComponentManager::~ComponentManager() {
	profiler::delete_channel(ch_component);
}



void ComponentManager::_register(Component *c) {
	auto& l = _get_list(c->component_type);
	l.add(c);

	auto family = get_component_type_family(c->component_type);
	auto& lf = _get_list_family(family);
	lf.add(c);
}

void ComponentManager::_unregister(Component *c) {
	auto& list = _get_list_x(this, c->component_type);
	list.remove(c);

	auto type_family = get_component_type_family(c->component_type);
	auto& flist = _get_list_x_family(this, type_family);
	flist.remove(c);
}


const kaba::Class *ComponentManager::get_component_type_family(const kaba::Class *type) {
	while (type->parent) {
		if (type->parent->name == "Component")
			return type;
		type = type->parent;
	}
	return type;
}

// TODO (later) optimize...
Component *ComponentManager::create_component(const kaba::Class *type, const string &var) {
	if (!factory)
		return nullptr;
	auto c = factory(type, var);
	c->component_type = type;
	_register(c);
	return c;
}

// should already be unlinked from entity!
void ComponentManager::delete_component(Component *c) {
	if (c->owner) {
		msg_error("trying to delete a component that is still attached to an entity");
		return;
	}
	_unregister(c);
	delete c;
}


ComponentManager::List &ComponentManager::_get_list(const kaba::Class *type) {
	return _get_list_x(this, type).list;
}

ComponentManager::List &ComponentManager::_get_list_family(const kaba::Class *type_family) {
	return _get_list_x_family(this, type_family).list;
}

void ComponentManager::iterate(float dt) {
	profiler::begin(ch_component);
	for (auto&& [type, list]: component_lists_by_type)
		if (list.needs_update) {
			profiler::begin(list.ch_iterate);
			for (auto *c: list.list)
				c->on_iterate(dt);
			profiler::end(list.ch_iterate);
		}
	profiler::end(ch_component);
}


ComponentManager::PairList& ComponentManager::_get_list2(const kaba::Class *type_a, const kaba::Class *type_b) {
	static PairList _list;

	// TODO cache
	_list.clear();
	for (auto c1: _get_list(type_a)) {
		if (auto c2 = c1->owner->_get_component_generic_(type_b))
			_list.add({c1->owner, c1, c2});
	}

	return _list;
}


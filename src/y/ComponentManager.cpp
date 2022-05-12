/*
 * ComponentManager.cpp
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#include "ComponentManager.h"
#include "Component.h"
#include "../lib/base/map.h"
#include "../lib/config.h"
#ifdef _X_ALLOW_X_
#include "../meta.h"
#include "../plugins/PluginManager.h"
#include "../helper/PerformanceMonitor.h"
#endif
#include "../lib/kaba/syntax/Class.h"
#include "../lib/kaba/syntax/Function.h"

#include "../lib/file/msg.h"

static int ch_component = -1;

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

Map<const kaba::Class*, ComponentListX> component_lists;

bool class_func_did_override(const kaba::Class *type, const string &fname) {
#ifdef _X_ALLOW_X_
	for (auto f: weak(type->functions))
		if (f->name == fname)
			return f->name_space != type->get_root();
#endif
	return false;
}

ComponentListX *get_list_x(const kaba::Class *type_family) {
	if (component_lists.find(type_family) >= 0) {
		return &component_lists[type_family];
	} else {
		ComponentListX list;
		list.type_family = type_family;
		list.needs_update = class_func_did_override(type_family, "on_iterate");
#ifdef _X_ALLOW_X_
		if (list.needs_update)
			list.ch_iterate = PerformanceMonitor::create_channel(type_family->long_name(), ch_component);
#endif
		component_lists.set(type_family, list);
		return &component_lists[type_family];
	}
}

void ComponentManager::init() {
#ifdef _X_ALLOW_X_
	ch_component = PerformanceMonitor::create_channel("component");
#endif
}


void ComponentManager::add_to_list(Component *c, const kaba::Class *type_family) {
	auto l = get_list_x(type_family);
	l->add(c);
}


const kaba::Class *ComponentManager::get_component_type_family(const kaba::Class *type) {
#ifdef _X_ALLOW_X_
	while (type->parent) {
		if ((type->parent->name == "Component") or (type->parent->name == "Component3D"))
			return type;
		type = type->parent;
	}
	return type;
#else
	return nullptr;
#endif
}

// TODO (later) optimize...
Component *ComponentManager::create_component(const kaba::Class *type, const string &var) {
#ifdef _X_ALLOW_X_
	//Component *c = nullptr;
	auto c = (Component*)PluginManager::create_instance(type, var);
	c->component_type = type;
	auto type_family = get_component_type_family(type);
	add_to_list(c, type_family);
	return c;
#else
	return nullptr;
#endif
}

void ComponentManager::delete_component(Component *c) {
	auto type_family = get_component_type_family(c->component_type);
	auto list = get_list_x(type_family);
	list->remove(c);
	delete c;
}


ComponentManager::List *ComponentManager::get_list(const kaba::Class *type_family) {
	return &get_list_x(type_family)->list;
}

void ComponentManager::iterate(float dt) {
#ifdef _X_ALLOW_X_
	PerformanceMonitor::begin(ch_component);
#endif
	for (auto &l: component_lists)
		if (l.value.needs_update) {
#ifdef _X_ALLOW_X_
			PerformanceMonitor::begin(l.value.ch_iterate);
#endif
			for (auto *c: l.value.list)
				c->on_iterate(dt);
#ifdef _X_ALLOW_X_
			PerformanceMonitor::end(l.value.ch_iterate);
#endif
		}
#ifdef _X_ALLOW_X_
	PerformanceMonitor::end(ch_component);
#endif
}


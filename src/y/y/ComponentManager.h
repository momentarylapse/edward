/*
 * ComponentManager.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <functional>

class Entity;
class Component;
class ComponentListX;
namespace kaba {
	class Class;
}


class ComponentManager {
public:
	using List = Array<Component*>;

	ComponentManager();
	~ComponentManager();

	Component *create_component(const kaba::Class *type, const string &var);
	void delete_component(Component *c);
	List &_get_list_family(const kaba::Class *type_family);
	List &_get_list(const kaba::Class *type);

	struct ComponentPair {
		Entity *e;
		Component *a, *b;
	};
	using PairList = Array<ComponentPair>;
	PairList& _get_list2(const kaba::Class *type_a, const kaba::Class *type_b);

	template<class C>
	Array<C*>& get_list() {
		return (Array<C*>&) _get_list(C::_class);
	}

	template<class C>
	Array<C*>& get_list_family() {
		return (Array<C*>&) _get_list_family(C::_class);
	}

	void _register(Component *c);
	void _unregister(Component *c);

	const kaba::Class *get_component_type_family(const kaba::Class *type);

	void iterate(float dt);

	base::map<const kaba::Class*, ComponentListX> component_lists_by_type;
	base::map<const kaba::Class*, ComponentListX> component_lists_by_family;
	int ch_component = -1;

	std::function<Component*(const kaba::Class*, const string& var)> factory;
};


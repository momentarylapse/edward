/*
 * ComponentManager.h
 *
 *  Created on: Jul 13, 2021
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>

class Component;
namespace kaba {
	class Class;
}


class ComponentManager {
public:
	using List = Array<Component*>;

	static void init();

	static Component *create_component(const kaba::Class *type, const string &var);
	static void delete_component(Component *c);
	static List *_get_list_family(const kaba::Class *type_family);
	static List *_get_list(const kaba::Class *type);

	template<class C>
	static Array<C*> *get_list() {
		return (Array<C*>*) _get_list(C::_class);
	}

	template<class C>
	static Array<C*> *get_list_family() {
		return (Array<C*>*) _get_list_family(C::_class);
	}

	static void _register(Component *c);

	static const kaba::Class *get_component_type_family(const kaba::Class *type);

	static void iterate(float dt);
};


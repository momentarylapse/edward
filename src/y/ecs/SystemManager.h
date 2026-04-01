//
// Created by Michael Ankele on 2024-10-13.
//

#pragma once

#include <lib/base/base.h>

class Path;
namespace kaba {
	class Class;
}

namespace ecs {

class EntityManager;
class System;
struct InstanceDataVariable;

class SystemManager {
public:
	static void init(int ch_iter, EntityManager* entity_manager);

	static void reset();

	static void create(const Path& filename, const string& name, const Array<InstanceDataVariable>& variables);
	static System* _get_generic(const kaba::Class* _class);
	template<class T>
	static T* get() {
		return (T*)_get_generic(T::_class);
	}
	static void register_system(const kaba::Class* _class, System* s);

	static void handle_finished_loading();
	static void handle_iterate_pre(float dt);
	static void handle_iterate(float dt);
	static void handle_input();
	static void handle_draw_pre();
	static void handle_render_inject();
	static void handle_render_inject2();

	static Array<System*> systems;
	static EntityManager* entity_manager;
};

}

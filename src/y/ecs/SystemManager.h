//
// Created by Michael Ankele on 2024-10-13.
//

#pragma once

#include <lib/base/base.h>

class Path;
class System;
struct ScriptInstanceDataVariable;
namespace kaba {
	class Class;
}

class SystemManager {
public:
	static void init(int ch_iter);

	static void reset();

	static void create(const Path& filename, const string& name, const Array<ScriptInstanceDataVariable>& variables);
	static System* get(const kaba::Class* _class);
	static void add_external(const kaba::Class* _class, System* s, bool immortal);

	static void handle_iterate_pre(float dt);
	static void handle_iterate(float dt);
	static void handle_input();
	static void handle_draw_pre();
	static void handle_render_inject();
	static void handle_render_inject2();

	static Array<System*> systems;
};


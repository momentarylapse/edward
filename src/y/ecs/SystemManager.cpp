//
// Created by Michael Ankele on 2024-10-13.
//

#include "SystemManager.h"
#include "System.h"
#include "../plugins/PluginManager.h"
#include <lib/profiler/Profiler.h>
#include <lib/kaba/kaba.h>
#include <lib/os/path.h>
#include <lib/os/msg.h>


Array<System*> SystemManager::systems;
static base::map<const kaba::Class*, System*> immprtal_systems;
static base::map<const kaba::Class*, System*> system_by_type;
static int ch_system = -1;
static int ch_con_iter_pre = -1;
static int ch_con_input = -1;
static int ch_con_draw_pre = -1;

void SystemManager::init(int ch_iter_parent) {
	ch_system = profiler::create_channel("sys", ch_iter_parent);
	ch_con_iter_pre = profiler::create_channel("it0", ch_iter_parent);
	ch_con_input = profiler::create_channel("in0", ch_iter_parent);
	ch_con_draw_pre = profiler::create_channel("dr0", ch_iter_parent);
}

void SystemManager::reset() {
	msg_write("del systems");
	for (auto&& [c, s]: system_by_type)
		if (!immprtal_systems.contains(c))
			delete s;
	systems.clear();
	system_by_type.clear();
	for (auto&& [c, s]: immprtal_systems) {
		systems.add(s);
		system_by_type.add({c, s});
	}
}

void SystemManager::create(const Path& filename, const string& __name, const Array<ScriptInstanceDataVariable> &variables) {
	msg_write("add system: " + filename.str());
	auto type = PluginManager::find_class_derived(filename, "ecs.System");
	auto* s = reinterpret_cast<System*>(PluginManager::create_instance(type, variables));
	s->ch_iterate = profiler::create_channel(type->long_name(), ch_system);

	systems.add(s);
	system_by_type.add({type, s});
	s->on_init();
}

void SystemManager::add_external(const kaba::Class* type, System* s, bool immortal) {
	systems.add(s);
	system_by_type.add({type, s});
	if (immortal)
		immprtal_systems.add({type, s});
}


System *SystemManager::get(const kaba::Class* type) {
	for (auto&& [t, s]: system_by_type)
		if (t == type)
			return s;
	return nullptr;
}

void SystemManager::handle_iterate(float dt) {
	profiler::begin(ch_system);
	for (auto *c: systems) {
		profiler::begin(c->ch_iterate);
		c->on_iterate(dt);
		profiler::end(c->ch_iterate);
	}
	profiler::end(ch_system);
}

void SystemManager::handle_iterate_pre(float dt) {
	profiler::begin(ch_con_iter_pre);
	for (auto *c: systems)
		c->on_iterate_pre(dt);
	profiler::end(ch_con_iter_pre);
}

void SystemManager::handle_input() {
	profiler::begin(ch_con_input);
	for (auto *c: systems)
		c->on_input();
	profiler::end(ch_con_input);
}

void SystemManager::handle_draw_pre() {
	profiler::begin(ch_con_draw_pre);
	for (auto *c: systems)
		c->on_draw_pre();
	profiler::end(ch_con_draw_pre);
}

void SystemManager::handle_render_inject() {
	for (auto *c: systems)
		c->on_render_inject();
}

void SystemManager::handle_render_inject2() {
	for (auto *c: systems)
		c->on_render_inject2();
}



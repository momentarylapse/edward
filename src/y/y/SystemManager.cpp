//
// Created by Michael Ankele on 2024-10-13.
//

#include "SystemManager.h"
#include "System.h"
#include "../plugins/PluginManager.h"
#include "../helper/PerformanceMonitor.h"
#include "../lib/kaba/kaba.h"
#include "../lib/os/path.h"
#include "../lib/os/msg.h"


Array<System*> SystemManager::systems;
static int ch_system = -1;
static int ch_con_iter_pre = -1;
static int ch_con_input = -1;
static int ch_con_draw_pre = -1;

void SystemManager::init(int ch_iter_parent) {
	ch_system = PerformanceMonitor::create_channel("sys", ch_iter_parent);
	ch_con_iter_pre = PerformanceMonitor::create_channel("it0", ch_iter_parent);
	ch_con_input = PerformanceMonitor::create_channel("in0", ch_iter_parent);
	ch_con_draw_pre = PerformanceMonitor::create_channel("dr0", ch_iter_parent);
}

void SystemManager::reset() {
	msg_write("del controller");
	for (auto *c: systems)
		delete c;
	systems.clear();
}

void SystemManager::create(const Path& filename, const string& __name, const Array<TemplateDataScriptVariable> &variables) {
	msg_write("add system: " + filename.str());
	auto type = PluginManager::find_class_derived(filename, "ui.Controller");
	auto *c = reinterpret_cast<System*>(PluginManager::create_instance(type, variables));
	c->_class = type;
	c->ch_iterate = PerformanceMonitor::create_channel(type->long_name(), ch_system);

	systems.add(c);
	c->on_init();
}

System *SystemManager::get(const kaba::Class *type) {
	for (auto c: systems)
		if (c->_class == type)
			return c;
	return nullptr;
}

void SystemManager::handle_iterate(float dt) {
	PerformanceMonitor::begin(ch_system);
	for (auto *c: systems) {
		PerformanceMonitor::begin(c->ch_iterate);
		c->on_iterate(dt);
		PerformanceMonitor::end(c->ch_iterate);
	}
	PerformanceMonitor::end(ch_system);
}

void SystemManager::handle_iterate_pre(float dt) {
	PerformanceMonitor::begin(ch_con_iter_pre);
	for (auto *c: systems)
		c->on_iterate_pre(dt);
	PerformanceMonitor::end(ch_con_iter_pre);
}

void SystemManager::handle_input() {
	PerformanceMonitor::begin(ch_con_input);
	for (auto *c: systems)
		c->on_input();
	PerformanceMonitor::end(ch_con_input);
}

void SystemManager::handle_draw_pre() {
	PerformanceMonitor::begin(ch_con_draw_pre);
	for (auto *c: systems)
		c->on_draw_pre();
	PerformanceMonitor::end(ch_con_draw_pre);
}

void SystemManager::handle_render_inject() {
	for (auto *c: systems)
		c->on_render_inject();
}

void SystemManager::handle_render_inject2() {
	for (auto *c: systems)
		c->on_render_inject2();
}



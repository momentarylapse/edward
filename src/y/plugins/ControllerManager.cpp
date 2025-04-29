//
// Created by Michael Ankele on 2024-10-13.
//

#include "ControllerManager.h"
#include "Controller.h"
#include "PluginManager.h"
#include "../helper/PerformanceMonitor.h"
#include "../lib/kaba/kaba.h"
#include "../lib/os/path.h"
#include "../lib/os/msg.h"


Array<Controller*> ControllerManager::controllers;
static int ch_controller = -1;
static int ch_con_iter_pre = -1;
static int ch_con_input = -1;
static int ch_con_draw_pre = -1;

void ControllerManager::init(int ch_iter_parent) {
	ch_controller = PerformanceMonitor::create_channel("controller", ch_iter_parent);
	ch_con_iter_pre = PerformanceMonitor::create_channel("it0", ch_iter_parent);
	ch_con_input = PerformanceMonitor::create_channel("in0", ch_iter_parent);
	ch_con_draw_pre = PerformanceMonitor::create_channel("dr0", ch_iter_parent);
}

void ControllerManager::reset() {
	msg_write("del controller");
	for (auto *c: controllers)
		delete c;
	controllers.clear();
}

void ControllerManager::add_controller(const Path& filename, const string& __name, const Array<TemplateDataScriptVariable> &variables) {
	msg_write("add controller: " + filename.str());
	auto type = PluginManager::find_class_derived(filename, "ui.Controller");
	auto *c = (Controller*)PluginManager::create_instance(type, variables);
	c->_class = type;
	c->ch_iterate = PerformanceMonitor::create_channel(type->long_name(), ch_controller);

	controllers.add(c);
	c->on_init();
}

Controller *ControllerManager::get_controller(const kaba::Class *type) {
	for (auto c: controllers)
		if (c->_class == type)
			return c;
	return nullptr;
}

void ControllerManager::handle_iterate(float dt) {
	PerformanceMonitor::begin(ch_controller);
	for (auto *c: controllers) {
		PerformanceMonitor::begin(c->ch_iterate);
		c->on_iterate(dt);
		PerformanceMonitor::end(c->ch_iterate);
	}
	PerformanceMonitor::end(ch_controller);
}

void ControllerManager::handle_iterate_pre(float dt) {
	PerformanceMonitor::begin(ch_con_iter_pre);
	for (auto *c: controllers)
		c->on_iterate_pre(dt);
	PerformanceMonitor::end(ch_con_iter_pre);
}

void ControllerManager::handle_input() {
	PerformanceMonitor::begin(ch_con_input);
	for (auto *c: controllers)
		c->on_input();
	PerformanceMonitor::end(ch_con_input);
}

void ControllerManager::handle_draw_pre() {
	PerformanceMonitor::begin(ch_con_draw_pre);
	for (auto *c: controllers)
		c->on_draw_pre();
	PerformanceMonitor::end(ch_con_draw_pre);
}

void ControllerManager::handle_render_inject() {
	for (auto *c: controllers)
		c->on_render_inject();
}

void ControllerManager::handle_render_inject2() {
	for (auto *c: controllers)
		c->on_render_inject2();
}



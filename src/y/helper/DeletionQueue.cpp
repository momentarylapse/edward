/*
 * DeletionQueue.cpp
 *
 *  Created on: 12 Mar 2023
 *      Author: michi
 */

#include "DeletionQueue.h"
#include "../world/World.h"
#include "../gui/gui.h"
#include "../lib/os/msg.h"

namespace DeletionQueue {

static Array<ecs::Entity*> queue_entity;
static Array<gui::Node*> queue_ui;

void add(ecs::BaseClass *c) {
	msg_error("this delete operation is deprecated... call Mi :(");
}

void add_entity(ecs::Entity *e) {
	queue_entity.add(e);
}

void add_ui(gui::Node *n) {
	queue_ui.add(n);
}

void delete_all() {
	for (auto e: queue_entity)
		world.delete_entity(e);
	for (auto n: queue_ui)
		gui::delete_node(n);
	reset();
}

void reset() {
	queue_entity.clear();
	queue_ui.clear();
}

}

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

static Array<BaseClass*> queue;
static Array<gui::Node*> queue_ui;

void add(BaseClass *c) {
	queue.add(c);
}

void add_ui(gui::Node *n) {
	queue_ui.add(n);
}

void delete_all() {
	for (auto c: queue) {
		if (c->type == BaseClass::Type::ENTITY)
			world.delete_entity(static_cast<Entity*>(c));
		else
			msg_error(format("unable to delete: %d", (int)c->type));
	}
	for (auto n: queue_ui)
		gui::delete_node(n);
	reset();
}

void reset() {
	queue.clear();
	queue_ui.clear();
}

}

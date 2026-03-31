/*
 * DeletionQueue.h
 *
 *  Created on: 12 Mar 2023
 *      Author: michi
 */

#ifndef SRC_HELPER_DELETIONQUEUE_H_
#define SRC_HELPER_DELETIONQUEUE_H_

namespace ecs {
	class BaseClass;
	struct Entity;
}
namespace gui {
	class Node;
}

namespace DeletionQueue {
	void add(ecs::BaseClass* c);
	void add_entity(ecs::Entity* e);
	void add_ui(gui::Node* n);
	void delete_all();
	void reset();
};

#endif /* SRC_HELPER_DELETIONQUEUE_H_ */

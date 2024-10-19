/*
 * DeletionQueue.h
 *
 *  Created on: 12 Mar 2023
 *      Author: michi
 */

#ifndef SRC_HELPER_DELETIONQUEUE_H_
#define SRC_HELPER_DELETIONQUEUE_H_

class BaseClass;
namespace gui {
	class Node;
}

namespace DeletionQueue {
	void add(BaseClass *c);
	void add_ui(gui::Node *n);
	void delete_all();
	void reset();
};

#endif /* SRC_HELPER_DELETIONQUEUE_H_ */

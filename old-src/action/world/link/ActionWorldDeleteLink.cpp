/*
 * ActionWorldDeleteLink.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldDeleteLink.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldLink.h"

ActionWorldDeleteLink::ActionWorldDeleteLink(int i) {
	index = i;
}

void* ActionWorldDeleteLink::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	link = w->links[index];
	w->links.erase(index);
	return nullptr;
}

void ActionWorldDeleteLink::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->links.insert(link, index);
}

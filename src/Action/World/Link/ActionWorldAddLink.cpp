/*
 * ActionWorldAddLink.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "ActionWorldAddLink.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldLink.h"

ActionWorldAddLink::ActionWorldAddLink(WorldLink &l) {
	link = l;
}

void* ActionWorldAddLink::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->links.add(link);
	return &w->links.back();
}

void ActionWorldAddLink::undo(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	w->links.pop();
}

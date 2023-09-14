/*
 * BaseClass.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "BaseClass.h"

Array<BaseClass*> EntityManager::selection;



BaseClass::BaseClass(Type t) {
	type = t;
}





void EntityManager::reset() {
	selection.clear();
}


void EntityManager::delete_later(BaseClass *p) {
	selection.add(p);
}

void EntityManager::delete_selection() {
	for (auto *p: selection)
		delete p;
	selection.clear();
}


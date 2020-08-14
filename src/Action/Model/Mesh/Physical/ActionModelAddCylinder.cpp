/*
 * ActionModelAddCylinder.cpp
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#include "ActionModelAddCylinder.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelAddCylinder::ActionModelAddCylinder(const ModelCylinder &c) {
	cylinder = c;
}

void* ActionModelAddCylinder::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->cylinder.add(cylinder);
	return &m->edit_mesh->cylinder.back();
}

void ActionModelAddCylinder::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->cylinder.pop();
}

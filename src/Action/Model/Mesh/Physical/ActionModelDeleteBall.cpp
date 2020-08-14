/*
 * ActionModelDeleteBall.cpp
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#include "ActionModelDeleteBall.h"
#include "../../../../Data/Model/DataModel.h"

ActionModelDeleteBall::ActionModelDeleteBall(int _index) {
	index = _index;
}

void* ActionModelDeleteBall::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	ball = m->edit_mesh->ball[index];
	m->edit_mesh->ball.erase(index);
	return nullptr;
}

void ActionModelDeleteBall::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->ball.insert(ball, index);
}

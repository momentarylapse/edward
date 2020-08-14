/*
 * ActionModelAddBall.cpp
 *
 *  Created on: Aug 14, 2020
 *      Author: michi
 */

#include "ActionModelAddBall.h"
#include "../../../../Data/Model/DataModel.h"


ActionModelAddBall::ActionModelAddBall(const ModelBall &b) {
	ball = b;
}

void* ActionModelAddBall::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->ball.add(ball);
	return &m->edit_mesh->ball.back();
}

void ActionModelAddBall::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	m->edit_mesh->ball.pop();
}

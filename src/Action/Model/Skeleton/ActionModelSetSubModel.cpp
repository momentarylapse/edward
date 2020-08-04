/*
 * ActionModelSetSubModel.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: ankele
 */

#include "ActionModelSetSubModel.h"
#include "../../../Data/Model/DataModel.h"
#include <assert.h>

#include "../../../x/ModelManager.h"

ActionModelSetSubModel::ActionModelSetSubModel(int _index, const Path &_filename) {
	index = _index;
	filename = _filename;
	model = ModelManager::load(filename);
}

ActionModelSetSubModel::~ActionModelSetSubModel() {
	if (model)
		delete model;
}

void* ActionModelSetSubModel::execute(Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->bone.num);

	ModelBone &b = m->bone[index];

	Model *tm = b.model;
	b.model = model;
	model = tm;

	auto tf = b.model_file;
	b.model_file = filename;
	filename = tf;

	return b.model;
}

void ActionModelSetSubModel::undo(Data* d) {
	execute(d);
}

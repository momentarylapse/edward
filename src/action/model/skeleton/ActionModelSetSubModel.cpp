/*
 * ActionModelSetSubModel.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: ankele
 */

#include "ActionModelSetSubModel.h"
#include "../../../data/model/DataModel.h"
#include <assert.h>

#include "../../../y/ModelManager.h"

ActionModelSetSubModel::ActionModelSetSubModel(int _index, const Path &_filename, Model *m) {
	index = _index;
	filename = _filename;
	model = m;
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

	std::swap(b.model, model);
	std::swap(b.model_file, filename);

	return b.model;
}

void ActionModelSetSubModel::undo(Data* d) {
	execute(d);
}

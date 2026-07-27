/*
 * ActionModelSetSubModel.cpp
 *
 *  Created on: Dec 16, 2014
 *      Author: ankele
 */

#include "ActionModelSetSubModel.h"
#include "../../data/DataModel.h"
#include <cassert>

ActionModelSetSubModel::ActionModelSetSubModel(int _index, const Path &_filename, Model *m) {
	index = _index;
	filename = _filename;
	model = m;
}

ActionModelSetSubModel::~ActionModelSetSubModel() {
	if (model)
		delete model;
}

void* ActionModelSetSubModel::execute(history::Data* d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->bones.num);

	ModelBone &b = m->bones[index];

	std::swap(b.model, model);
	std::swap(b.model_file, filename);

	return b.model;
}

void ActionModelSetSubModel::undo(history::Data* d) {
	execute(d);
}

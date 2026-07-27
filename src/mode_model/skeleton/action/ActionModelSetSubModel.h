/*
 * ActionModelSetSubModel.h
 *
 *  Created on: Dec 16, 2014
 *      Author: ankele
 */

#pragma once

#include <lib/history/Action.h>
//#include <lib/math/vec3.h>

class Model;

class ActionModelSetSubModel: public history::Action {
public:
	ActionModelSetSubModel(int index, const Path &filename, Model *m);
	~ActionModelSetSubModel() override;
	string name() const override { return "ModelSetSubModel"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	Path filename;
	Model *model;
};

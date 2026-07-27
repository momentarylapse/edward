/*
 * ActionModelAddBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#pragma once

#include <lib/history/Action.h>
#include <lib/math/vec3.h>

class ActionModelAddBone: public history::Action {
public:
	ActionModelAddBone(const vec3& pos, int parent);
	string name() const override { return "ModelAddBone"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	vec3 pos;
	int parent;
};

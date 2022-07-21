/*
 * ActionWorldAddTerrain.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDADDTERRAIN_H_
#define ACTIONWORLDADDTERRAIN_H_

#include "../../Action.h"
#include "../../../lib/math/vector.h"
#include "../../../lib/os/path.h"

class ActionWorldAddTerrain : public Action {
public:
	ActionWorldAddTerrain(const vector &_pos, const Path &_filename);
	ActionWorldAddTerrain(const vector &_pos, const vector &_size, int _num_x, int _num_z);
	string name(){	return "WorldAddTerrain";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	Path filename;
	vector pos;
	vector size;
	int num_x, num_z;
};

#endif /* ACTIONWORLDADDTERRAIN_H_ */

/*
 * ActionWorldAddTerrain.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDADDTERRAIN_H_
#define ACTIONWORLDADDTERRAIN_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionWorldAddTerrain : public Action
{
public:
	ActionWorldAddTerrain(const vector &_pos, const vector &_size, int _num_x, int _num_z);
	virtual ~ActionWorldAddTerrain();
	string name(){	return "WorldAddTerrain";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos;
	vector size;
	int num_x, num_z;
};

#endif /* ACTIONWORLDADDTERRAIN_H_ */

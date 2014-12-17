/*
 * ActionModelAnimationSetData.cpp
 *
 *  Created on: 17.12.2014
 *      Author: michi
 */

#include <assert.h>
#include <algorithm>
#include "ActionModelAnimationSetData.h"
#include "../../../Data/Model/DataModel.h"

ActionModelAnimationSetData::ActionModelAnimationSetData(int _index, const string &_name, float _fps_const, float _fps_factor)
{
	index = _index;
	mname = _name;
	fps_const = _fps_const;
	fps_factor = _fps_factor;
}

void *ActionModelAnimationSetData::execute(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert(index >= 0);
	assert(index < m->move.num);
	ModelMove &mv = m->move[index];

	std::swap(mname, mv.name);
	std::swap(fps_const, mv.frames_per_sec_const);
	std::swap(fps_factor, mv.frames_per_sec_factor);

	return NULL;
}

void ActionModelAnimationSetData::undo(Data *d)
{
	execute(d);
}




/*
 * ActionModelAutomap.h
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#ifndef ACTIONMODELAUTOMAP_H_
#define ACTIONMODELAUTOMAP_H_

#include "../../../Action.h"
#include "../../../../x/material.h"
#include "../../../../lib/math/math.h"
class DataModel;

class MultiView;

class ActionModelAutomap : public Action
{
public:
	ActionModelAutomap(int material, int texture_level);
	virtual ~ActionModelAutomap();
	string name(){	return "ModelAutoMap";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material;
	int texture_level;
	Array<vector> old_pos[MATERIAL_MAX_TEXTURES];
};

#endif /* ACTIONMODELAUTOMAP_H_ */

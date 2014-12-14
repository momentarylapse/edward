/*
 * ActionModelAutomap.h
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#ifndef ACTIONMODELAUTOMAP_H_
#define ACTIONMODELAUTOMAP_H_

#include "../../../Action.h"
#include "../../../../lib/math/math.h"
class DataModel;

class ActionModelAutomap : public Action
{
public:
	ActionModelAutomap(int material, int texture_level);
	virtual ~ActionModelAutomap();
	virtual string name(){	return "ModelAutoMap";	}
	virtual const string &message();

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	int material;
	int texture_level;
	Array<vector> old_pos;
};

#endif /* ACTIONMODELAUTOMAP_H_ */

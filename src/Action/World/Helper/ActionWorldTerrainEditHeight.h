/*
 * ActionWorldTerrainEditHeight.h
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINEDITHEIGHT_H_
#define ACTIONWORLDTERRAINEDITHEIGHT_H_

#include "../../Action.h"
#include "../../../lib/hui/hui.h"

class ActionWorldTerrainEditHeight : public Action
{
public:
	ActionWorldTerrainEditHeight(int _index, const Array<float> &_height, irect _dest);
	virtual ~ActionWorldTerrainEditHeight();
	string name(){	return "WorldTerrainEditHeight";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	Array<float> height;
	irect dest;
};

#endif /* ACTIONWORLDTERRAINEDITHEIGHT_H_ */

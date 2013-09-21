/*
 * ActionWorldTerrainEditHeight.h
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINEDITHEIGHT_H_
#define ACTIONWORLDTERRAINEDITHEIGHT_H_

#include "../../Action.h"
#include "../../../lib/math/rect.h"

class ActionWorldTerrainEditHeight : public Action
{
public:
	ActionWorldTerrainEditHeight(int _index, const Array<float> &_height, rect _dest);
	virtual ~ActionWorldTerrainEditHeight();
	string name(){	return "WorldTerrainEditHeight";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	Array<float> height;
	rect dest;
};

#endif /* ACTIONWORLDTERRAINEDITHEIGHT_H_ */

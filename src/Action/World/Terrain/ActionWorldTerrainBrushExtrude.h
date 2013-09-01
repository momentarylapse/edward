/*
 * ActionWorldTerrainBrushExtrude.h
 *
 *  Created on: 01.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINBRUSHEXTRUDE_H_
#define ACTIONWORLDTERRAINBRUSHEXTRUDE_H_

#include "../../Action.h"
#include "../../../Data/World/DataWorld.h"

class ActionWorldTerrainBrushExtrude : public Action
{
public:
	ActionWorldTerrainBrushExtrude(int _index, const vector &pos, float radius, float depth);
	virtual ~ActionWorldTerrainBrushExtrude();
	string name(){	return "WorldTerrainBrushExtrude";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	vector pos;
	float radius, depth;
	int index;
	Array<int> v_index;
	Array<float> old_height;
};

#endif /* ACTIONWORLDTERRAINBRUSHEXTRUDE_H_ */

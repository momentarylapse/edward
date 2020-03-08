/*
 * ActionWorldTerrainBrushExtrude.h
 *
 *  Created on: 01.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINBRUSHEXTRUDE_H_
#define ACTIONWORLDTERRAINBRUSHEXTRUDE_H_

#include "../../Action.h"
#include "../../../lib/math/vector.h"

class ActionWorldTerrainBrushExtrude : public Action {
public:
	ActionWorldTerrainBrushExtrude(int _index, const vector &pos, float radius, float depth);
	string name(){	return "WorldTerrainBrushExtrude";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	vector pos;
	float radius, depth;
	int index;
	Array<int> v_index;
	Array<float> old_height;
};

#endif /* ACTIONWORLDTERRAINBRUSHEXTRUDE_H_ */

/*
 * ActionWorldTerrainBrushExtrude.h
 *
 *  Created on: 01.09.2013
 *      Author: michi
 */

#ifndef ACTIONWORLDTERRAINBRUSHEXTRUDE_H_
#define ACTIONWORLDTERRAINBRUSHEXTRUDE_H_

#include "../../../lib/math/vec3.h"
#include "../../Action.h"

class ActionWorldTerrainBrushExtrude : public Action {
public:
	ActionWorldTerrainBrushExtrude(int _index, const vec3 &pos, float radius, float depth);
	string name(){	return "WorldTerrainBrushExtrude";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	vec3 pos;
	float radius, depth;
	int index;
	Array<int> v_index;
	Array<float> old_height;
};

#endif /* ACTIONWORLDTERRAINBRUSHEXTRUDE_H_ */

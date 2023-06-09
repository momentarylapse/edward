/*
 * ActionModelAutomap.h
 *
 *  Created on: 12.05.2013
 *      Author: michi
 */

#ifndef ACTIONMODELAUTOMAP_H_
#define ACTIONMODELAUTOMAP_H_

#include "../../../../lib/math/vec3.h"
#include "../../../Action.h"
class DataModel;

class ActionModelAutomap : public Action {
public:
	ActionModelAutomap(int material, int texture_level);
	string name() override { return "ModelAutoMap"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int material;
	int texture_level;
	Array<vec3> old_pos;
};

#endif /* ACTIONMODELAUTOMAP_H_ */

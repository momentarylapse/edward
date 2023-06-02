/*
 * ActionModelEditEffect.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITEFFECT_H_
#define ACTIONMODELEDITEFFECT_H_

#include "../../../Action.h"
#include "../../../../data/model/DataModel.h"

class ActionModelEditEffect : public Action
{
public:
	ActionModelEditEffect(int _index, const ModelEffect &_effect);
	virtual ~ActionModelEditEffect(){}
	string name(){	return "ModelEditEffect";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	ModelEffect effect;
	int index;
};

#endif /* ACTIONMODELEDITEFFECT_H_ */

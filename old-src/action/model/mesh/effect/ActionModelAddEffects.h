/*
 * ActionModelAddEffects.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDEFFECTS_H_
#define ACTIONMODELADDEFFECTS_H_

#include "../../../Action.h"
#include "../../../../data/model/DataModel.h"

class ActionModelAddEffects : public Action
{
public:
	ActionModelAddEffects(DataModel *m, const ModelEffect &_effect);
	virtual ~ActionModelAddEffects(){}
	string name(){	return "ModelAddEffects";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	ModelEffect effect;
	Array<int> vertex;
};

#endif /* ACTIONMODELADDEFFECTS_H_ */

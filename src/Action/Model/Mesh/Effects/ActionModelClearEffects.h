/*
 * ActionModelClearEffects.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef ACTIONMODELCLEAREFFECTS_H_
#define ACTIONMODELCLEAREFFECTS_H_

#include "../../../Action.h"
#include "../../../../Data/Model/DataModel.h"

class ActionModelClearEffects : public Action
{
public:
	ActionModelClearEffects(DataModel *m);
	virtual ~ActionModelClearEffects(){}
	string name(){	return "ModelClearEffects";	}

	void *execute(Data *d);
	void undo(Data *d);
private:
	Array<ModelEffect> effects;
	Array<int> index;
};

#endif /* ACTIONMODELCLEAREFFECTS_H_ */

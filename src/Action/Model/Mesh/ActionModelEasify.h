/*
 * ActionModelEasify.h
 *
 *  Created on: 18.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEASIFY_H_
#define ACTIONMODELEASIFY_H_

#include "../../ActionGroup.h"
class DataModel;

class ActionModelEasify: public ActionGroup
{
public:
	ActionModelEasify(float factor);
	virtual ~ActionModelEasify();
	string name(){	return "ModelEasify";	}

	void *compose(Data *d);

private:
	float factor;
	void CalculateWeights(DataModel *m);
	bool EasifyStep(DataModel *m);
};

#endif /* ACTIONMODELEASIFY_H_ */

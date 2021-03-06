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
class ModelMesh;

class ActionModelEasify: public ActionGroup
{
public:
	ActionModelEasify(float factor);
	string name(){	return "ModelEasify";	}

	void *compose(Data *d);

private:
	float factor;
	void CalculateWeights(ModelMesh *m);
	bool EasifyStep(ModelMesh *m);
};

#endif /* ACTIONMODELEASIFY_H_ */

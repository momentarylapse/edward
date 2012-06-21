/*
 * ActionModelEasify.h
 *
 *  Created on: 18.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEASIFY_H_
#define ACTIONMODELEASIFY_H_

#include "../../ActionGroup.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelEasify: public ActionGroup
{
public:
	ActionModelEasify(DataModel *m, float factor);
	virtual ~ActionModelEasify();
	string name(){	return "ModelEasify";	}
};

#endif /* ACTIONMODELEASIFY_H_ */

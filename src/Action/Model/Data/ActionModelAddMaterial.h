/*
 * ActionModelAddMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDMATERIAL_H_
#define ACTIONMODELADDMATERIAL_H_

#include "../../Action.h"

class ActionModelAddMaterial : public Action
{
public:
	ActionModelAddMaterial(const string &filename);
	virtual ~ActionModelAddMaterial();
	virtual string name(){	return "ModelAddMaterial";	}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);
private:
	string filename;
};

#endif /* ACTIONMODELADDMATERIAL_H_ */

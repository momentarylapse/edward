/*
 * ActionModelSetMaterial.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETMATERIAL_H_
#define ACTIONMODELSETMATERIAL_H_

#include "../../../Action.h"
class DataModel;

class ActionModelSetMaterial: public Action
{
public:
	ActionModelSetMaterial(DataModel *m, int _material);
	string name(){	return "ModelSetMaterial";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int material;
	Array<int> triangle;
	Array<int> old_material;
};

#endif /* ACTIONMODELSETMATERIAL_H_ */

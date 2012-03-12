/*
 * ActionModelSetMaterial.h
 *
 *  Created on: 12.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSETMATERIAL_H_
#define ACTIONMODELSETMATERIAL_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModelSetMaterial: public Action
{
public:
	ActionModelSetMaterial(int _material);
	virtual ~ActionModelSetMaterial();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

private:
	int material;
	Array<int> surface;
	Array<int> triangle;
	Array<int> old_material;
};

#endif /* ACTIONMODELSETMATERIAL_H_ */

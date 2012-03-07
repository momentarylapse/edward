/*
 * ActionModelAddTrianglesByOutline.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDTRIANGLESBYOUTLINE_H_
#define ACTIONMODELADDTRIANGLESBYOUTLINE_H_

#include "../Action.h"
#include "../ActionGroup.h"
#include "../../Data/Model/DataModel.h"
#include "../../lib/types/types.h"

class ActionModelAddTrianglesByOutline: public ActionGroup
{
public:
	ActionModelAddTrianglesByOutline(Array<int> vertex, DataModel *data);
	virtual ~ActionModelAddTrianglesByOutline();

	bool needs_preparation();
	void prepare(Data *d);
};

#endif /* ACTIONMODELADDTRIANGLESBYOUTLINE_H_ */

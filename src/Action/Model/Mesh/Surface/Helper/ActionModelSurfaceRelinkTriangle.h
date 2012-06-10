/*
 * ActionModelSurfaceRelinkTriangle.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACERELINKTRIANGLE_H_
#define ACTIONMODELSURFACERELINKTRIANGLE_H_

#include "../../../../Action.h"
#include "../../../../ActionGroup.h"
#include "../../../../../lib/types/types.h"
#include "../../../../../Data/Model/DataModel.h"

class ActionModelSurfaceRelinkTriangle : public ActionGroup
{
public:
	ActionModelSurfaceRelinkTriangle(DataModel *m, int _surface, int _triangle, int a, int b, int c);
	virtual ~ActionModelSurfaceRelinkTriangle();
	string name(){	return "ModelSurfaceRelinkTriangle";	}
};

#endif /* ACTIONMODELSURFACERELINKTRIANGLE_H_ */

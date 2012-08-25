/*
 * ActionModelSurfaceRelinkTriangle.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACERELINKTRIANGLE_H_
#define ACTIONMODELSURFACERELINKTRIANGLE_H_

#include "../../../../ActionGroup.h"
class DataModel;

class ActionModelSurfaceRelinkTriangle : public ActionGroup
{
public:
	ActionModelSurfaceRelinkTriangle(DataModel *m, int _surface, int _triangle, int a, int b, int c);
	virtual ~ActionModelSurfaceRelinkTriangle();
	string name(){	return "ModelSurfaceRelinkTriangle";	}
};

#endif /* ACTIONMODELSURFACERELINKTRIANGLE_H_ */

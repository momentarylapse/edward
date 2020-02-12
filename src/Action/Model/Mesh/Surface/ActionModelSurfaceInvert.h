/*
 * ActionModelSurfaceInvert.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEINVERT_H_
#define ACTIONMODELSURFACEINVERT_H_

#include "../../../Action.h"
#include "../../../../lib/base/set.h"
class Data;
class DataModel;

class ActionModelSurfaceInvert : public Action
{
public:
	ActionModelSurfaceInvert(const Set<int> &surfaces);
	string name(){	return "ModelSurfaceInvert";	}

	void *execute(Data *d);
	void undo(Data *d);

	virtual bool was_trivial();

private:
	void InvertSurface(DataModel *m);
};

#endif /* ACTIONMODELSURFACEINVERT_H_ */

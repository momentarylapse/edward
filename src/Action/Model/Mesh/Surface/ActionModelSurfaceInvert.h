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
class ModelSurface;

class ActionModelSurfaceInvert : public Action
{
public:
	ActionModelSurfaceInvert(const Set<int> &surfaces);
	virtual ~ActionModelSurfaceInvert();
	string name(){	return "ModelSurfaceInvert";	}

	void *execute(Data *d);
	void undo(Data *d);

	virtual bool was_trivial();

private:
	Set<int> surfaces;

	void InvertSurface(ModelSurface &s);
};

#endif /* ACTIONMODELSURFACEINVERT_H_ */

/*
 * ActionModelSurfaceInvert.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEINVERT_H_
#define ACTIONMODELSURFACEINVERT_H_

#include "../../../Action.h"
#include "../../../../Data/Data.h"

class ActionModelSurfaceInvert : public Action
{
public:
	ActionModelSurfaceInvert(int _surface);
	virtual ~ActionModelSurfaceInvert();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
};

#endif /* ACTIONMODELSURFACEINVERT_H_ */

/*
 * ActionModelSurfaceInvert.h
 *
 *  Created on: 03.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSURFACEINVERT_H_
#define ACTIONMODELSURFACEINVERT_H_

#include "../../../Action.h"
class Data;

class ActionModelSurfaceInvert : public Action
{
public:
	ActionModelSurfaceInvert(int _surface);
	virtual ~ActionModelSurfaceInvert();
	string name(){	return "ModelSurfacenvert";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
};

#endif /* ACTIONMODELSURFACEINVERT_H_ */

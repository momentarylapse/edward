/*
 * ActionModelDeleteEmptySurface.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELDELETEEMPTYSURFACE_H_
#define ACTIONMODELDELETEEMPTYSURFACE_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModelDeleteEmptySurface: public Action
{
public:
	ActionModelDeleteEmptySurface(int _surface);
	virtual ~ActionModelDeleteEmptySurface();
	string name(){	return "ModelDeleteEmptySurface";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
	bool is_physical, is_visible;
};

#endif /* ACTIONMODELDELETEEMPTYSURFACE_H_ */

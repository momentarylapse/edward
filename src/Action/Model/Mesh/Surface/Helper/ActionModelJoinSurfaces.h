/*
 * ActionModelJoinSurfaces.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELJOINSURFACES_H_
#define ACTIONMODELJOINSURFACES_H_

#include "../../../../Action.h"

class ActionModelJoinSurfaces: public Action
{
public:
	ActionModelJoinSurfaces(int _surface1, int _surface2);
	virtual ~ActionModelJoinSurfaces();
	string name(){	return "ModelJoinSurface";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface1, surface2;
	int old_edges1, old_trias1, old_vertices1;
};

#endif /* ACTIONMODELJOINSURFACES_H_ */

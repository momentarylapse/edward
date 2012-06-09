/*
 * ActionModel__JoinSurfaces.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__JOINSURFACES_H_
#define ACTIONMODEL__JOINSURFACES_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModel__JoinSurfaces: public Action
{
public:
	ActionModel__JoinSurfaces(int _surface1, int _surface2);
	virtual ~ActionModel__JoinSurfaces();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface1, surface2;
	int old_edges1, old_trias1, old_vertices1;
};

#endif /* ACTIONMODEL__JOINSURFACES_H_ */

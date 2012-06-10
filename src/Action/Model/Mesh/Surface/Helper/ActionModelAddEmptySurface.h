/*
 * ActionModelAddEmptySurface.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELADDEMPTYSURFACE_H_
#define ACTIONMODELADDEMPTYSURFACE_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModelAddEmptySurface: public Action
{
public:
	ActionModelAddEmptySurface();
	virtual ~ActionModelAddEmptySurface();
	string name(){	return "ModelAddEmptySurface";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
};

#endif /* ACTIONMODELADDEMPTYSURFACE_H_ */

/*
 * ActionModel__AddSurface.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__ADDSURFACE_H_
#define ACTIONMODEL__ADDSURFACE_H_

#include "../../../../Action.h"
#include "../../../../../lib/types/types.h"

class ActionModel__AddSurface: public Action
{
public:
	ActionModel__AddSurface();
	virtual ~ActionModel__AddSurface();

	void *execute(Data *d);
	void undo(Data *d);

private:
};

#endif /* ACTIONMODEL__ADDSURFACE_H_ */

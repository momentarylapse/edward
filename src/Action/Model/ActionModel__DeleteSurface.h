/*
 * ActionModel__DeleteSurface.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODEL__DELETESURFACE_H_
#define ACTIONMODEL__DELETESURFACE_H_

#include "../Action.h"
#include "../../lib/types/types.h"

class ActionModel__DeleteSurface: public Action
{
public:
	ActionModel__DeleteSurface(int _surface);
	virtual ~ActionModel__DeleteSurface();

	void *execute(Data *d);
	void undo(Data *d);

private:
	int surface;
	bool is_physical, is_visible;
};

#endif /* ACTIONMODEL__DELETESURFACE_H_ */

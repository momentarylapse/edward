/*
 * ActionWorldRotateObjects.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDROTATEOBJECTS_H_
#define ACTIONWORLDROTATEOBJECTS_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class ActionWorldRotateObjects: public ActionMultiView
{
public:
	ActionWorldRotateObjects(Data *d, const vector &_pos0);
	virtual ~ActionWorldRotateObjects();
	string name(){	return "WorldRotateObjects";	}

	void *execute(Data *d);
	void undo(Data *d);

	// continuous editing
	void abort(Data *d);
private:
	Array<vector> old_ang;
};

#endif /* ACTIONWORLDROTATEOBJECTS_H_ */

/*
 * ActionWorldMoveObjects.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDMOVEOBJECTS_H_
#define ACTIONWORLDMOVEOBJECTS_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class ActionWorldMoveObjects: public ActionMultiView
{
public:
	ActionWorldMoveObjects(Data *d, const vector &_pos0);
	virtual ~ActionWorldMoveObjects();
	string name(){	return "WorldMoveObjects";	}

	void *execute(Data *d);
	void undo(Data *d);

	// continuous editing
	void abort(Data *d);
};

#endif /* ACTIONWORLDMOVEOBJECTS_H_ */

/*
 * ActionModelMoveBones.h
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMOVEBONES_H_
#define ACTIONMODELMOVEBONES_H_

#include "../../ActionMultiView.h"
#include "../../../Data/Data.h"
#include "../../../lib/file/file.h"
#include "../../../lib/types/types.h"

class ActionModelMoveBones: public ActionMultiView
{
public:
	ActionModelMoveBones(Data *d, const vector &_pos0);
	virtual ~ActionModelMoveBones();
	string name(){	return "ModelMoveBones";	}

	void *execute(Data *d);
	void undo(Data *d);
};

#endif /* ACTIONMODELMOVEBONES_H_ */

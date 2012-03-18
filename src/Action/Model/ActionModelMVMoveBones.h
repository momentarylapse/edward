/*
 * ActionModelMVMoveBones.h
 *
 *  Created on: 18.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMOVEBONES_H_
#define ACTIONMODELMVMOVEBONES_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class ActionModelMVMoveBones: public ActionMultiView
{
public:
	ActionModelMVMoveBones(Data *d, const vector &_pos0);
	virtual ~ActionModelMVMoveBones();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	// continuous editing
	void abort(Data *d);
};

#endif /* ACTIONMODELMVMOVEBONES_H_ */

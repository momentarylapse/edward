/*
 * ActionModelMVMoveVertices.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMOVEVERTICES_H_
#define ACTIONMODELMVMOVEVERTICES_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class Data;

class ActionModelMVMoveVertices: public ActionMultiView
{
public:
	ActionModelMVMoveVertices(Data *d, int _set_no, const Array<int> &_index);
	virtual ~ActionModelMVMoveVertices();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	// continuous editing
	void abort(Data *d);
	void set_param(Data *d, const vector &_param);
};

#endif /* ACTIONMODELMVMOVEVERTICES_H_ */

/*
 * ActionModelMVRotateVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVROTATEVERTICES_H_
#define ACTIONMODELMVROTATEVERTICES_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class Data;

class ActionModelMVRotateVertices: public ActionMultiView
{
public:
	ActionModelMVRotateVertices(Data *d, int _set_no, const Array<int> &_index, const vector &_pos0);
	virtual ~ActionModelMVRotateVertices();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	// continuous editing
	void abort(Data *d);
	void set_param(Data *d, const vector &_param);
};

#endif /* ACTIONMODELMVROTATEVERTICES_H_ */

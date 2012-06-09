/*
 * ActionModelMVMirrorVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVMIRRORVERTICES_H_
#define ACTIONMODELMVMIRRORVERTICES_H_

#include "../../../ActionMultiView.h"
#include "../../../../Data/Data.h"
#include "../../../../lib/file/file.h"
#include "../../../../lib/types/types.h"

class Data;

class ActionModelMVMirrorVertices: public ActionMultiView
{
public:
	ActionModelMVMirrorVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVMirrorVertices();

	void *execute(Data *d);
	void undo(Data *d);

	// continuous editing
	void abort(Data *d);
};

#endif /* ACTIONMODELMVMIRRORVERTICES_H_ */

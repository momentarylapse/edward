/*
 * ActionModelMVScaleVertices.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef ACTIONMODELMVSCALEVERTICES_H_
#define ACTIONMODELMVSCALEVERTICES_H_

#include "../ActionMultiView.h"
#include "../../Data/Data.h"
#include "../../lib/file/file.h"
#include "../../lib/types/types.h"

class Data;

class ActionModelMVScaleVertices: public ActionMultiView
{
public:
	ActionModelMVScaleVertices(Data *d, const vector &_pos0);
	virtual ~ActionModelMVScaleVertices();

	void *execute(Data *d);
	void undo(Data *d);
	void redo(Data *d);

	// continuous editing
	void abort(Data *d);
	void set_param(Data *d, const vector &_param);
	void set_axis(const vector &_e1, const vector &_e2, const vector &_e3);
private:
	vector e[3];
};

#endif /* ACTIONMODELMVSCALEVERTICES_H_ */

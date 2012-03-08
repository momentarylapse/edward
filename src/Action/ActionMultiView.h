/*
 * ActionMultiView.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMULTIVIEW_H_
#define ACTIONMULTIVIEW_H_

#include "Action.h"
#include "../Data/Data.h"
#include "../lib/file/file.h"
#include "../lib/types/types.h"

class Data;

class ActionMultiView: public Action
{
public:
	ActionMultiView(int _set_no, Array<int> _index);
	virtual ~ActionMultiView();

	virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d) = 0;

	// continuous editing
	virtual void abort(Data *d) = 0;
	virtual void set_param(const vector &_param) = 0;

protected:
	int set_no;
	Array<int> index;
	vector param;
	Array<vector> old_data;
};

#endif /* ACTIONMULTIVIEW_H_ */

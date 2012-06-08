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
	ActionMultiView(Data *d, const vector &_pos0);
	virtual ~ActionMultiView();

	/*virtual void *execute(Data *d) = 0;
	virtual void undo(Data *d) = 0;
	virtual void redo(Data *d) = 0;*/

	// continuous editing
	virtual void abort(Data *d) = 0;
	//virtual void set_param(Data *d, const vector &_param) = 0;
	virtual void set_axis(const vector &_e1, const vector &_e2, const vector &_e3);
	void abort_and_notify(Data *d);
	void set_param_and_notify(Data *d, const vector &_param);

protected:
	Array<int> index;
	vector pos0, param;
	Array<vector> old_data;
	vector e[3];
};

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, const vector &_pos0);

#endif /* ACTIONMULTIVIEW_H_ */

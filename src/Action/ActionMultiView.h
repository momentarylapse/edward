/*
 * ActionMultiView.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMULTIVIEW_H_
#define ACTIONMULTIVIEW_H_

#include "Action.h"
class Data;
#include "../lib/file/file.h"
#include "../lib/types/types.h"

class Data;

class ActionMultiView: public Action
{
public:
	ActionMultiView(const vector &_param, const vector &_pos0);
	virtual ~ActionMultiView();

	// continuous editing
	virtual void abort(Data *d);
	void abort_and_notify(Data *d);

protected:
	Array<int> index;
	vector pos0;
	vector param;
	Array<vector> old_data;
};

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, const vector &_param, const vector &_pos0, const vector &_ex, const vector &_ey, const vector &_ez);

#endif /* ACTIONMULTIVIEW_H_ */

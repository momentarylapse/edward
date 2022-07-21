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
#include "../lib/math/matrix.h"

class Data;

class ActionMultiView: public Action {
public:
	ActionMultiView();

	// continuous editing
	virtual void abort(Data *d);
	void abort_and_notify(Data *d);
	void update_and_notify(Data *d, const matrix &m);

protected:
	Array<int> index;
	matrix mat;
	Array<vector> old_data;
};

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d);

#endif /* ACTIONMULTIVIEW_H_ */

/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "Model/ActionModelMVMoveVertices.h"

ActionMultiView::ActionMultiView(Data *d, int _set_no, const Array<int> &_index)
{
	set_no = _set_no;
	index = _index;
}

ActionMultiView::~ActionMultiView()
{
}

void ActionMultiView::set_param_and_notify(Data *d, const vector & _param)
{
	set_param(d, _param);
	d->Notify();
}



void ActionMultiView::abort_and_notify(Data *d)
{
	abort(d);
	d->Notify();
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, int _set_no, const Array<int> &_index)
{
	if (name == "ActionModelMVMoveVertices")
		return new ActionModelMVMoveVertices(d, _set_no, _index);
	return NULL;
}



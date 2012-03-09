/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "Model/ActionModelMVMoveVertices.h"
#include "Model/ActionModelMVScaleVertices.h"
#include "Model/ActionModelMVRotateVertices.h"
#include "Model/ActionModelMVMirrorVertices.h"

ActionMultiView::ActionMultiView(Data *d, int _set_no, const Array<int> &_index, const vector &_pos0)
{
	set_no = _set_no;
	index = _index;
	pos0 = _pos0;
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

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, int _set_no, const Array<int> &_index, const vector &_pos0)
{
	if (name == "ActionModelMVMoveVertices")
		return new ActionModelMVMoveVertices(d, _set_no, _index, _pos0);
	if (name == "ActionModelMVScaleVertices")
		return new ActionModelMVScaleVertices(d, _set_no, _index, _pos0);
	if (name == "ActionModelMVRotateVertices")
		return new ActionModelMVRotateVertices(d, _set_no, _index, _pos0);
	if (name == "ActionModelMVMirrorVertices")
		return new ActionModelMVMirrorVertices(d, _set_no, _index, _pos0);
	return NULL;
}



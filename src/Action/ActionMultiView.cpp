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

ActionMultiView::ActionMultiView(Data *d, const vector &_pos0)
{
	pos0 = _pos0;
}

ActionMultiView::~ActionMultiView()
{
}




void ActionMultiView::set_axis(const vector &_e1, const vector &_e2, const vector &_e3)
{
	e[0] = _e1;
	e[1] = _e2;
	e[2] = _e3;
}


void ActionMultiView::set_param_and_notify(Data *d, const vector & _param)
{
	set_param(d, _param);
	d->Notify("Change");
}



void ActionMultiView::abort_and_notify(Data *d)
{
	abort(d);
	d->Notify("Change");
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, const vector &_pos0)
{
	if (name == "ActionModelMVMoveVertices")
		return new ActionModelMVMoveVertices(d, _pos0);
	if (name == "ActionModelMVScaleVertices")
		return new ActionModelMVScaleVertices(d, _pos0);
	if (name == "ActionModelMVRotateVertices")
		return new ActionModelMVRotateVertices(d, _pos0);
	if (name == "ActionModelMVMirrorVertices")
		return new ActionModelMVMirrorVertices(d, _pos0);
	return NULL;
}



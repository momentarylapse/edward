/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "Model/Mesh/Vertex/ActionModelMoveVertices.h"
#include "Model/Mesh/Vertex/ActionModelScaleVertices.h"
#include "Model/Mesh/Vertex/ActionModelRotateVertices.h"
#include "Model/Mesh/Vertex/ActionModelMirrorVertices.h"
#include "Model/Mesh/Skin/ActionModelMoveSkinVertices.h"
#include "Model/Mesh/Skin/ActionModelScaleSkinVertices.h"
#include "Model/Mesh/Skin/ActionModelRotateSkinVertices.h"
#include "Model/Mesh/Skin/ActionModelMirrorSkinVertices.h"
#include "Model/Skeleton/ActionModelMoveBones.h"
#include "Model/Animation/ActionModelAnimationRotateBones.h"
#include "Model/Animation/ActionModelAnimationMoveVertices.h"
#include "Model/Animation/ActionModelAnimationRotateVertices.h"
#include "World/ActionWorldMoveSelection.h"
#include "World/ActionWorldRotateObjects.h"
#include <assert.h>

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
	param = _param;
	execute(d);
	d->Notify("Change");
}



void ActionMultiView::abort(Data *d)
{
	undo(d);
}



void ActionMultiView::abort_and_notify(Data *d)
{
	abort(d);
	d->Notify("Change");
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, const vector &_pos0)
{
	if (name == "ActionModelMoveVertices")
		return new ActionModelMoveVertices(d, _pos0);
	else if (name == "ActionModelScaleVertices")
		return new ActionModelScaleVertices(d, _pos0);
	else if (name == "ActionModelRotateVertices")
		return new ActionModelRotateVertices(d, _pos0);
	else if (name == "ActionModelMirrorVertices")
		return new ActionModelMirrorVertices(d, _pos0);
	else if (name == "ActionModelMoveSkinVertices")
		return new ActionModelMoveSkinVertices(d, _pos0);
	else if (name == "ActionModelScaleSkinVertices")
		return new ActionModelScaleSkinVertices(d, _pos0);
	else if (name == "ActionModelRotateSkinVertices")
		return new ActionModelRotateSkinVertices(d, _pos0);
	else if (name == "ActionModelMirrorSkinVertices")
		return new ActionModelMirrorSkinVertices(d, _pos0);
	else if (name == "ActionModelMoveBones")
		return new ActionModelMoveBones(d, _pos0);
	else if (name == "ActionModelAnimationRotateBones")
		return new ActionModelAnimationRotateBones(d, _pos0);
	else if (name == "ActionModelAnimationMoveVertices")
		return new ActionModelAnimationMoveVertices(d, _pos0);
	else if (name == "ActionModelAnimationRotateVertices")
		return new ActionModelAnimationRotateVertices(d, _pos0);
	else if (name == "ActionWorldMoveSelection")
		return new ActionWorldMoveSelection(d, _pos0);
	else if (name == "ActionWorldRotateObjects")
		return new ActionWorldRotateObjects(d, _pos0);
	msg_error("ActionMultiViewFactory: unknown action: " + name);
	assert(0);
	return NULL;
}



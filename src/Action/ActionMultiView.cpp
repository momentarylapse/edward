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
#include "World/Camera/ActionCameraMoveSelection.h"
#include <assert.h>

ActionMultiView::ActionMultiView(const vector &_param, const vector &_pos0)
{
	param = _param;
	pos0 = _pos0;
}

ActionMultiView::~ActionMultiView()
{
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

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d, const vector &_param, const vector &_pos0, const vector &_ex, const vector &_ey, const vector &_ez)
{
	if (name == "ActionModelMoveVertices")
		return new ActionModelMoveVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelScaleVertices")
		return new ActionModelScaleVertices((DataModel*)d, _param, _pos0, _ex, _ey, _ez);
	else if (name == "ActionModelRotateVertices")
		return new ActionModelRotateVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelMirrorVertices")
		return new ActionModelMirrorVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelMoveSkinVertices")
		return new ActionModelMoveSkinVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelScaleSkinVertices")
		return new ActionModelScaleSkinVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelRotateSkinVertices")
		return new ActionModelRotateSkinVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelMirrorSkinVertices")
		return new ActionModelMirrorSkinVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelMoveBones")
		return new ActionModelMoveBones((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelAnimationRotateBones")
		return new ActionModelAnimationRotateBones((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelAnimationMoveVertices")
		return new ActionModelAnimationMoveVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelAnimationRotateVertices")
		return new ActionModelAnimationRotateVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionWorldMoveSelection")
		return new ActionWorldMoveSelection((DataWorld*)d, _param, _pos0);
	else if (name == "ActionWorldRotateObjects")
		return new ActionWorldRotateObjects((DataWorld*)d, _param, _pos0);
	else if (name == "ActionCameraMoveSelection")
		return new ActionCameraMoveSelection((DataCamera*)d, _param, _pos0);
	msg_error("ActionMultiViewFactory: unknown action: " + name);
	assert(0);
	return NULL;
}



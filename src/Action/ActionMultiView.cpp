/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "Model/Mesh/Vertex/ActionModelTransformVertices.h"
#include "Model/Mesh/Skin/ActionModelTransformSkinVertices.h"
#include "Model/Skeleton/ActionModelMoveBones.h"
#include "Model/Animation/ActionModelAnimationRotateBones.h"
#include "Model/Animation/ActionModelAnimationMoveVertices.h"
#include "Model/Animation/ActionModelAnimationRotateVertices.h"
#include "World/ActionWorldMoveSelection.h"
#include "World/Object/ActionWorldRotateObjects.h"
#include "World/Camera/ActionCameraMoveSelection.h"
#include "../Mode/Model/Mesh/ModeModelMeshTexture.h"
#include <assert.h>

ActionMultiView::ActionMultiView()
{
	mat = m_id;
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
	d->notify(message());
}

void ActionMultiView::update_and_notify(Data *d, const matrix &m)
{
	abort(d);
	mat = m;
	execute(d);
	d->notify(message());
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d)
{
	if (name == "ActionModelTransformVertices")
		return new ActionModelTransformVertices((DataModel*)d);
	if (name == "ActionModelTransformSkinVertices")
		return new ActionModelTransformSkinVertices((DataModel*)d, mode_model_mesh_texture->CurrentTextureLevel);
	else if (name == "ActionModelMoveBones")
		return new ActionModelMoveBones((DataModel*)d);
	/*else if (name == "ActionModelAnimationRotateBones")
		return new ActionModelAnimationRotateBones((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelAnimationMoveVertices")
		return new ActionModelAnimationMoveVertices((DataModel*)d, _param, _pos0);
	else if (name == "ActionModelAnimationRotateVertices")
		return new ActionModelAnimationRotateVertices((DataModel*)d, _param, _pos0);*/
	if (name == "ActionWorldMoveSelection")
		return new ActionWorldMoveSelection((DataWorld*)d);
	else if (name == "ActionWorldRotateObjects")
		return new ActionWorldRotateObjects((DataWorld*)d);
	/*else if (name == "ActionCameraMoveSelection")
		return new ActionCameraMoveSelection((DataCamera*)d, _param, _pos0);*/
	msg_error("ActionMultiViewFactory: unknown action: " + name);
	assert(0);
	return NULL;
}



/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "Model/Mesh/Vertex/ActionModelTransformVertices.h"
#include "Model/Mesh/Skin/ActionModelTransformSkinVertices.h"
#include "Model/Animation/ActionModelAnimationTransformVertices.h"
#include "Model/Animation/ActionModelAnimationTransformBones.h"
#include "World/ActionWorldMoveSelection.h"
#include "World/Object/ActionWorldRotateObjects.h"
#include "World/Camera/ActionCameraMoveSelection.h"
#include "../Mode/Model/Animation/ModeModelAnimation.h"
#include <assert.h>

#include "../Mode/Model/Mesh/ModeModelMeshTexture.h"
#include "Model/Skeleton/ActionModelTransformBones.h"

ActionMultiView::ActionMultiView() {
	mat = matrix::ID;
}



void ActionMultiView::abort(Data *d) {
	undo(d);
}



void ActionMultiView::abort_and_notify(Data *d) {
	abort(d);
	d->notify(message());
}

void ActionMultiView::update_and_notify(Data *d, const matrix &m) {
	abort(d);
	mat = m;
	execute(d);
	d->notify(message());
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d) {
	if (name == "ActionModelTransformVertices")
		return new ActionModelTransformVertices((DataModel*)d);
	if (name == "ActionModelTransformSkinVertices")
		return new ActionModelTransformSkinVertices((DataModel*)d, mode_model_mesh_texture->current_texture_level);
	if (name == "ActionModelTransformBones")
		return new ActionModelTransformBones((DataModel*)d);
	if (name == "ActionModelAnimationTransformBones")
		return new ActionModelAnimationTransformBones((DataModel*)d, mode_model_animation->current_move, mode_model_animation->current_frame);
	if (name == "ActionModelAnimationTransformVertices")
		return new ActionModelAnimationTransformVertices((DataModel*)d, mode_model_animation->current_move, mode_model_animation->current_frame);
	if (name == "ActionWorldMoveSelection")
		return new ActionWorldMoveSelection((DataWorld*)d);
	if (name == "ActionWorldRotateObjects")
		return new ActionWorldRotateObjects((DataWorld*)d);
	/*if (name == "ActionCameraMoveSelection")
		return new ActionCameraMoveSelection((DataCamera*)d, _param, _pos0);*/
	msg_error("ActionMultiViewFactory: unknown action: " + name);
	assert(0);
	return NULL;
}



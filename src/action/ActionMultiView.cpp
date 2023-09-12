/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"
#include "model/mesh/vertex/ActionModelTransformVertices.h"
#include "model/mesh/skin/ActionModelTransformSkinVertices.h"
#include "model/animation/ActionModelAnimationTransformVertices.h"
#include "model/animation/ActionModelAnimationTransformBones.h"
#include "model/skeleton/ActionModelTransformBones.h"
#include "world/ActionWorldMoveSelection.h"
#include "world/object/ActionWorldRotateObjects.h"
#include "world/camera/ActionCameraMoveSelection.h"
#include "../mode/model/ModeModel.h"
#include "../mode/model/animation/ModeModelAnimation.h"
#include "../mode/model/mesh/ModeModelMeshTexture.h"
#include "../Session.h"
#include <assert.h>

ActionMultiView::ActionMultiView() {
	mat = mat4::ID;
}



void ActionMultiView::abort(Data *d) {
	undo(d);
}



void ActionMultiView::abort_and_notify(Data *d) {
	abort(d);
	d->out_changed.notify();
}

void ActionMultiView::update_and_notify(Data *d, const mat4 &m) {
	abort(d);
	mat = m;
	execute(d);
	d->out_changed.notify();
}

ActionMultiView *ActionMultiViewFactory(const string &name, Data *d) {
	if (name == "ActionModelTransformVertices")
		return new ActionModelTransformVertices((DataModel*)d);
	if (name == "ActionModelTransformSkinVertices") {
		Array<int> tria;
		Array<int> index;
		d->session->find_mode<ModeModelMeshTexture>("model-mesh-texture")->getSelectedSkinVertices(tria, index);
		return new ActionModelTransformSkinVertices((DataModel*)d, tria, index, d->session->find_mode<ModeModelMeshTexture>("model-mesh-texture")->current_texture_level);
	}
	if (name == "ActionModelTransformBones")
		return new ActionModelTransformBones((DataModel*)d);
	if (name == "ActionModelAnimationTransformBones")
		return new ActionModelAnimationTransformBones((DataModel*)d, d->session->mode_model->mode_model_animation->current_move, d->session->mode_model->mode_model_animation->current_frame);
	if (name == "ActionModelAnimationTransformVertices")
		return new ActionModelAnimationTransformVertices((DataModel*)d, d->session->mode_model->mode_model_animation->current_move, d->session->mode_model->mode_model_animation->current_frame);
	if (name == "ActionWorldMoveSelection")
		return new ActionWorldMoveSelection((DataWorld*)d);
	if (name == "ActionWorldRotateObjects")
		return new ActionWorldRotateObjects((DataWorld*)d);
	/*if (name == "ActionCameraMoveSelection")
		return new ActionCameraMoveSelection((DataCamera*)d, _param, _pos0);*/
	msg_error("ActionMultiViewFactory: unknown action: " + name);
	assert(0);
	return nullptr;
}



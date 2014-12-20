/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../ModeModel.h"
#include "../../../../Action/Model/Skeleton/ActionModelAttachVerticesToBone.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../Mesh/MeshSelectionModePolygon.h"

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent,
		int _bone_index) :
		ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Vertices ausw&ahlen, [Return] = fertig");
	bone_index = _bone_index;
}

void ModeModelSkeletonAttachVertices::onStart()
{
	// relative to absolute pos
	foreach(ModelVertex &v, data->vertex)
		v.is_selected = (v.bone_index == bone_index);
	data->SelectionFromVertices();

	mode_model->allowSelectionModes(true);
	mode_model_mesh->selection_mode->updateMultiView();

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	onUpdate(data, "");
	onUpdate(multi_view, "");
}

void ModeModelSkeletonAttachVertices::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);

	parent->onUpdate(data, "");
}

void ModeModelSkeletonAttachVertices::onUpdate(Observable *o, const string &message)
{
	if (o == data){
		mode_model_mesh->selection_mode->updateMultiView();
	}else if (o == multi_view){
		mode_model_mesh->selection_mode->updateSelection();
		mode_model_mesh->fillSelectionBuffer(data->vertex);
	}
}

void ModeModelSkeletonAttachVertices::onKeyDown()
{
	int key = HuiGetEvent()->key_code;
	if (key == KEY_RETURN){
		Array<int> index;
		foreachi(ModelVertex &v, data->vertex, i)
			if (v.is_selected)
				index.add(i);
		data->execute(new ActionModelAttachVerticesToBone(index, bone_index));
		abort();
	}
}

void ModeModelSkeletonAttachVertices::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->onDrawWin(win);
}



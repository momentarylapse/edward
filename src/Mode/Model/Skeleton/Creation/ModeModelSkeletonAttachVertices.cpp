/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
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
		multi_view->clearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->addData(	MVD_MODEL_VERTEX,
				data->vertex,
				NULL,
				MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE);
	}else if (o == multi_view){
		data->SelectionFromVertices();
		mode_model_mesh->fillSelectionBuffers(data->vertex);
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
	mode_model_mesh->drawSelection(win);
}



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

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent,
		int _bone_index) :
		ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Vertices ausw&ahlen");
	bone_index = _bone_index;
}

ModeModelSkeletonAttachVertices::~ModeModelSkeletonAttachVertices()
{
}

void ModeModelSkeletonAttachVertices::onStart()
{
	// relative to absolute pos
	foreach(ModelVertex &v, data->Vertex)
		v.is_selected = (v.BoneIndex == bone_index);

	//Subscribe(data);
	subscribe(multi_view, "SelectionChange");

	multi_view->ClearData(data);
	multi_view->allow_rect = true;
	multi_view->AddData(	MVDModelVertex,
			data->Vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect);
	data->SelectionFromVertices();
	onUpdate(data);
}

void ModeModelSkeletonAttachVertices::onEnd()
{
	parent->onUpdate(data);
}

void ModeModelSkeletonAttachVertices::onKeyDown()
{
	int key = HuiGetEvent()->key_code;
	if (key == KEY_RETURN){
		Array<int> index;
		foreachi(ModelVertex &v, data->Vertex, i)
			if (v.is_selected)
				index.add(i);
		data->execute(new ActionModelAttachVerticesToBone(index, bone_index));
		abort();
	}
}

void ModeModelSkeletonAttachVertices::onDrawWin(MultiView::Window *win)
{
}



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

void ModeModelSkeletonAttachVertices::OnStart()
{
	// relative to absolute pos
	foreach(ModelVertex &v, data->Vertex)
		v.is_selected = (v.BoneIndex == bone_index);

	//Subscribe(data);
	Subscribe(multi_view, "SelectionChange");

	multi_view->ResetData(data);
	multi_view->allow_rect = true;
	multi_view->SetData(	MVDModelVertex,
			data->Vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
			NULL, NULL);
	data->SelectionFromVertices();
	OnUpdate(data);
}

void ModeModelSkeletonAttachVertices::OnEnd()
{
	parent->OnUpdate(data);
}

void ModeModelSkeletonAttachVertices::OnKeyDown()
{
	int key = HuiGetEvent()->key_code;
	if (key == KEY_RETURN){
		Array<int> index;
		foreachi(ModelVertex &v, data->Vertex, i)
			if (v.is_selected)
				index.add(i);
		data->Execute(new ActionModelAttachVerticesToBone(index, bone_index));
		Abort();
	}
}

void ModeModelSkeletonAttachVertices::OnDrawWin(MultiView::Window *win)
{
}



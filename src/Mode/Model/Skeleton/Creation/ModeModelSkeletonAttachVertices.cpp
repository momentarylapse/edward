/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../../../Edward.h"

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(Mode* _parent,
		int _bone_index) :
		ModeCreation("ModelSkeletonAttachVertices", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Vertices ausw&ahlen");
	bone_index = _bone_index;
}

ModeModelSkeletonAttachVertices::~ModeModelSkeletonAttachVertices()
{
}

void ModeModelSkeletonAttachVertices::OnStart()
{
	// relative to absolute pos
	foreach(data->Vertex, v)
		v.is_selected = (v.BoneIndex == bone_index);

	//Subscribe(data);
	Subscribe(multi_view, "SelectionChange");

	multi_view->ResetData(data);
	multi_view->MVRectable = true;
	multi_view->SetData(	MVDModelVertex,
			data->Vertex,
			NULL,
			MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect,
			NULL, NULL);
	OnUpdate(data);
}

void ModeModelSkeletonAttachVertices::OnEnd()
{
}

void ModeModelSkeletonAttachVertices::OnKeyDown()
{
}

void ModeModelSkeletonAttachVertices::OnDrawWin(int win, irect dest)
{
}



/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../Mesh/ModeModelMeshPolygon.h"
#include "../../../../Action/Model/Skeleton/ActionModelAttachVerticesToBone.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent,
		int _bone_index) :
		ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Vertices ausw&ahlen, [Return] = fertig");
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

	subscribe(data);
	subscribe(multi_view, "SelectionChange");
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
	if (o->getName() == "Data"){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->AddData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o->getName() == "MultiView"){
		data->SelectionFromVertices();
		mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
	}
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
	mode_model_mesh_polygon->DrawSelection(win);
}



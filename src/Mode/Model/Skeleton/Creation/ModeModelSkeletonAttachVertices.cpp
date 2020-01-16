/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../Mesh/Selection/MeshSelectionModePolygon.h"

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent, int _bone_index) :
	ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Vertices ausw&ahlen, [Ctrl + Return] = fertig");
	bone_index = _bone_index;
}

void ModeModelSkeletonAttachVertices::on_start()
{
	// relative to absolute pos
	for (ModelVertex &v: data->vertex)
		v.is_selected = (v.bone_index == bone_index);
	data->selectionFromVertices();

	//mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);

	mode_model->allow_selection_modes(true);
	mode_model_mesh->selection_mode->updateMultiView();
	multi_view->setAllowAction(false);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	on_update(data, "");
	on_update(multi_view, "");
}

void ModeModelSkeletonAttachVertices::on_end()
{
	unsubscribe(data);
	unsubscribe(multi_view);

	mode_model->allow_selection_modes(false);

	parent->on_update(data, "");
}

void ModeModelSkeletonAttachVertices::on_update(Observable *o, const string &message)
{
	if (o == data){
		mode_model_mesh->selection_mode->updateMultiView();
	}else if (o == multi_view){
		mode_model_mesh->selection_mode->updateSelection();
		mode_model_mesh->fill_selection_buffer(data->vertex);
	}
}

void ModeModelSkeletonAttachVertices::on_command(const string &id)
{
	if (id == "finish-action"){
		Array<int> index;
		foreachi(ModelVertex &v, data->vertex, i)
			if (v.is_selected)
				index.add(i);
		data->boneAttachVertices(bone_index, index);
		abort();
	}
}

void ModeModelSkeletonAttachVertices::on_draw_win(MultiView::Window *win)
{
	mode_model_mesh->on_draw_win(win);
}

void ModeModelSkeletonAttachVertices::on_set_multi_view()
{
	mode_model_mesh->on_set_multi_view();
}



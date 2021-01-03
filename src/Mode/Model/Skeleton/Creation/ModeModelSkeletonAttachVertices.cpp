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
#include "../../../../Data/Model/ModelMesh.h"
#include "../../Mesh/Selection/MeshSelectionModePolygon.h"

ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent, int _bone_index) :
	ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Select vertices, [Ctrl + Return] = done");
	bone_index = _bone_index;
}

void ModeModelSkeletonAttachVertices::on_start() {
	// relative to absolute pos
	for (auto &v: data->edit_mesh->vertex)
		v.is_selected = (v.bone_index == bone_index);
	data->selectionFromVertices();

	//mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);

	mode_model->allow_selection_modes(true);
	mode_model_mesh->selection_mode->update_multi_view();
	multi_view->set_allow_action(false);

	data->subscribe(this, [=]{ on_data_change(); });
	multi_view->subscribe(this, [=]{ on_update_selection(); }, multi_view->MESSAGE_SELECTION_CHANGE);

	on_data_change();
	on_update_selection();
}

void ModeModelSkeletonAttachVertices::on_end() {
	data->unsubscribe(this);
	multi_view->unsubscribe(this);

	mode_model->allow_selection_modes(false);

	//parent->on_update(data, "");
}

void ModeModelSkeletonAttachVertices::on_data_change() {
	mode_model_mesh->selection_mode->update_multi_view();
}

void ModeModelSkeletonAttachVertices::on_update_selection() {
	mode_model_mesh->selection_mode->on_update_selection();
	mode_model_mesh->fill_selection_buffer(data->edit_mesh->vertex);
}

void ModeModelSkeletonAttachVertices::on_command(const string &id) {
	if (id == "finish-action") {
		Array<int> index;
		foreachi(auto &v, data->edit_mesh->vertex, i)
			if (v.is_selected)
				index.add(i);
		data->boneAttachVertices(bone_index, index);
		abort();
	}
}

void ModeModelSkeletonAttachVertices::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->on_draw_win(win);
}

void ModeModelSkeletonAttachVertices::on_set_multi_view() {
	mode_model_mesh->on_set_multi_view();
}



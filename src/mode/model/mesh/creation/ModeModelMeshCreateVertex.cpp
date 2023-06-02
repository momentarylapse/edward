/*
 * ModeModelMeshCreateVertex.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateVertex.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../multiview/MultiView.h"

ModeModelMeshCreateVertex::ModeModelMeshCreateVertex(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateVertex", _parent)
{}

void ModeModelMeshCreateVertex::on_start() {
	message = _("place new vertex");

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_vertex);
	ed->mode_model->allow_selection_modes(false);
}


void ModeModelMeshCreateVertex::on_left_button_down() {
	data->addVertex(multi_view->get_cursor());
}

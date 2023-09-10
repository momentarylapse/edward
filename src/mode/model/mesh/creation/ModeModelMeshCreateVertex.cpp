/*
 * ModeModelMeshCreateVertex.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreateVertex.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../EdwardWindow.h"
#include "../../../../multiview/MultiView.h"

ModeModelMeshCreateVertex::ModeModelMeshCreateVertex(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshCreateVertex", _parent)
{}

void ModeModelMeshCreateVertex::on_start() {
	message = _("place new vertex");

	parent->set_selection_mode(parent->selection_mode_vertex);
	ed->mode_model->allow_selection_modes(false);
}


void ModeModelMeshCreateVertex::on_left_button_down() {
	data->addVertex(multi_view->get_cursor());
}

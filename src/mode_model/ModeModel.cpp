//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"
#include "mesh/ModeMesh.h"
#include <Session.h>
#include <view/MultiView.h>

Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false);

ModeModel::ModeModel(Session* session) : Mode(session) {
	multi_view = new MultiView(session);
	data = new DataModel(session);
	generic_data = data.get();

	mode_mesh = new ModeMesh(this);
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter() {
}

void ModeModel::on_leave() {
}









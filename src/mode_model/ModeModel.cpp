//
// Created by Michael Ankele on 2025-02-07.
//

#include "ModeModel.h"

ModeModel::ModeModel(Session* session) : Mode(session) {
	data = new DataModel(session);
}

ModeModel::~ModeModel() = default;

void ModeModel::on_enter() {
}




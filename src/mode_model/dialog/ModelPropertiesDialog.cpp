//
// Created by Michael Ankele on 2025-03-05.
//

#include "ModelPropertiesDialog.h"

#include <mode_model/data/DataModel.h>

ModelPropertiesDialog::ModelPropertiesDialog(xhui::Panel* parent, DataModel* _data) : Dialog("model_dialog", parent) {
	data = _data;

	fill();
}

void ModelPropertiesDialog::fill() {
	set_float("mass", data->meta_data.mass);
	check("ph_active", data->meta_data.active_physics);
	check("ph_passive", data->meta_data.passive_physics);

	add_string("tensor", format("X\\%.2f\\%.2f\\%.2f", data->meta_data.inertia_tensor.e[0], data->meta_data.inertia_tensor.e[1], data->meta_data.inertia_tensor.e[2]));
	add_string("tensor", format("Y\\%.2f\\%.2f\\%.2f", data->meta_data.inertia_tensor.e[3], data->meta_data.inertia_tensor.e[4], data->meta_data.inertia_tensor.e[5]));
	add_string("tensor", format("Z\\%.2f\\%.2f\\%.2f", data->meta_data.inertia_tensor.e[6], data->meta_data.inertia_tensor.e[7], data->meta_data.inertia_tensor.e[8]));
}



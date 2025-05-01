//
// Created by Michael Ankele on 2025-05-02.
//

#include "ActionMaterialEditAppearance.h"

ActionMaterialEditAppearance::ActionMaterialEditAppearance(const DataMaterial::AppearanceData& _data) {
	data = _data;
}

void* ActionMaterialEditAppearance::execute(Data* d) {
	auto m = static_cast<DataMaterial*>(d);

	std::swap(m->appearance, data);
	return nullptr;
}

void ActionMaterialEditAppearance::undo(Data* d) {
	execute(d);
}




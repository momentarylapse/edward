//
// Created by Michael Ankele on 2025-05-02.
//

#include "ActionMaterialEditAppearance.h"

ActionMaterialEditAppearance::ActionMaterialEditAppearance(const yrenderer::Material& m) {
	data = m;
}

void* ActionMaterialEditAppearance::execute(history::Data* d) {
	auto m = static_cast<DataMaterial*>(d);

	std::swap(m->material, data);
	return nullptr;
}

void ActionMaterialEditAppearance::undo(history::Data* d) {
	execute(d);
}




/*
 * ActionMaterialEditAppearance.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionMaterialEditAppearance.h"
#include <algorithm>

ActionMaterialEditAppearance::ActionMaterialEditAppearance(const DataMaterial::AppearanceData &_app) {
	app = _app;
}

void *ActionMaterialEditAppearance::execute(Data *d) {
	DataMaterial *m = dynamic_cast<DataMaterial*>(d);

	std::swap(app, m->appearance);

	return NULL;
}



void ActionMaterialEditAppearance::undo(Data *d) {
	execute(d);
}



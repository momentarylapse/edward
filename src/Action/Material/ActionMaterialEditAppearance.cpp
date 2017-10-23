/*
 * ActionMaterialEditAppearance.cpp
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#include "ActionMaterialEditAppearance.h"

ActionMaterialEditAppearance::ActionMaterialEditAppearance(const DataMaterial::AppearanceData &_app)
{
	app = _app;
}

ActionMaterialEditAppearance::~ActionMaterialEditAppearance()
{
}

void *ActionMaterialEditAppearance::execute(Data *d)
{
	DataMaterial *m = dynamic_cast<DataMaterial*>(d);

	DataMaterial::AppearanceData old_app = m->appearance;

	m->appearance = app;

	app = old_app;

	return NULL;
}



void ActionMaterialEditAppearance::undo(Data *d)
{
	execute(d);
}



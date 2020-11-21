/*
 * ActionMaterialEditAppearance.h
 *
 *  Created on: 07.06.2012
 *      Author: michi
 */

#ifndef ACTIONMATERIALEDITAPPEARANCE_H_
#define ACTIONMATERIALEDITAPPEARANCE_H_

#include "../Action.h"
#include "../../Data/Material/DataMaterial.h"

class ActionMaterialEditAppearance : public Action {
public:
	ActionMaterialEditAppearance(const DataMaterial::AppearanceData &_app);
	string name(){	return "MaterialEditAppearance";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	DataMaterial::AppearanceData app;
};

#endif /* ACTIONMATERIALEDITAPPEARANCE_H_ */

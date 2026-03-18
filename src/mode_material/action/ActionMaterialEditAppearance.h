//
// Created by Michael Ankele on 2025-05-02.
//

#ifndef ACTIONMATERIALEDITAPPEARANCE_H
#define ACTIONMATERIALEDITAPPEARANCE_H


#include <action/Action.h>
#include "../data/DataMaterial.h"

class ActionMaterialEditAppearance : public Action {
public:
	explicit ActionMaterialEditAppearance(const yrenderer::Material& m);
	string name() override { return "MaterialEditAppearance"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	yrenderer::Material data;
};


#endif //ACTIONMATERIALEDITAPPEARANCE_H

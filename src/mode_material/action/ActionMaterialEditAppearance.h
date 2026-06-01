//
// Created by Michael Ankele on 2025-05-02.
//

#ifndef ACTIONMATERIALEDITAPPEARANCE_H
#define ACTIONMATERIALEDITAPPEARANCE_H


#include <lib/history/Action.h>
#include "../data/DataMaterial.h"

class ActionMaterialEditAppearance : public history::Action {
public:
	explicit ActionMaterialEditAppearance(const yrenderer::Material& m);
	string name() const override { return "MaterialEditAppearance"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	yrenderer::Material data;
};


#endif //ACTIONMATERIALEDITAPPEARANCE_H

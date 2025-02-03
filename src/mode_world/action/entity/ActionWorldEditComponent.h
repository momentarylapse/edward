//
// Created by Michael Ankele on 2025-02-03.
//

#ifndef ACTIONWORLDEDITCOMPONENT_H
#define ACTIONWORLDEDITCOMPONENT_H


#include "../../../action/Action.h"
#include "../../data/WorldLight.h"

class ActionWorldEditLight : public Action {
public:
	explicit ActionWorldEditLight(int index, const WorldLight &l);
	string name() override { return "WorldEditLight"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldLight light;
};

class ActionWorldEditComponent {

};



#endif //ACTIONWORLDEDITCOMPONENT_H

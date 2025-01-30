//
// Created by Michael Ankele on 2025-01-31.
//

#ifndef ACTIONWORLDADDENTITY_H
#define ACTIONWORLDADDENTITY_H


#include "../../../action/Action.h"
#include "../../data/DataWorld.h"

class ActionWorldAddEntity : public Action {
public:
	explicit ActionWorldAddEntity(const WorldEntity& e);
	string name() override { return "WorldAddEntity"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	WorldEntity entity;
};



#endif //ACTIONWORLDADDENTITY_H

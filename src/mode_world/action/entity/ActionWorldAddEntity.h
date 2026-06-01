//
// Created by Michael Ankele on 2025-01-31.
//

#ifndef ACTIONWORLDADDENTITY_H
#define ACTIONWORLDADDENTITY_H


#include <lib/history/Action.h>
#include "../../data/DataWorld.h"

class ActionWorldAddEntity : public history::Action {
public:
	explicit ActionWorldAddEntity(const vec3& pos, const quaternion& ang);
	string name() const override { return "WorldAddEntity"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	vec3 pos;
	quaternion ang;
	ecs::Entity* entity = nullptr;
};



#endif //ACTIONWORLDADDENTITY_H

//
// Created by Michael Ankele on 2025-04-21.
//

#ifndef ACTIONWORLDEDITTERRAINMETADATA_H
#define ACTIONWORLDEDITTERRAINMETADATA_H



#include <action/Action.h>
#include <lib/math/vec3.h>

class ActionWorldEditTerrainMetaData : public Action {
public:
	ActionWorldEditTerrainMetaData(int index, const vec3& pattern);
	string name() override { return "WorldEditTerrainMetaData";	}

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	vec3 pattern;
};



#endif //ACTIONWORLDEDITTERRAINMETADATA_H

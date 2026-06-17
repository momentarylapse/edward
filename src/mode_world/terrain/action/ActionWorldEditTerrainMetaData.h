//
// Created by Michael Ankele on 2025-04-21.
//

#ifndef ACTIONWORLDEDITTERRAINMETADATA_H
#define ACTIONWORLDEDITTERRAINMETADATA_H



#include <lib/history/Action.h>
#include <lib/math/vec3.h>

class Terrain;

class ActionWorldEditTerrainMetaData : public history::Action {
public:
	ActionWorldEditTerrainMetaData(Terrain* t, const vec3& pattern, const vec3 texture_scale[8]);
	string name() const override { return "WorldEditTerrainMetaData";	}

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Terrain* terrain;
	vec3 pattern;
	vec3 texture_scale[8];
};



#endif //ACTIONWORLDEDITTERRAINMETADATA_H

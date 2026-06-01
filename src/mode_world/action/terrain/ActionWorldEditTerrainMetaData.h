//
// Created by Michael Ankele on 2025-04-21.
//

#ifndef ACTIONWORLDEDITTERRAINMETADATA_H
#define ACTIONWORLDEDITTERRAINMETADATA_H



#include <lib/history/Action.h>
#include <lib/math/vec3.h>

class ActionWorldEditTerrainMetaData : public history::Action {
public:
	ActionWorldEditTerrainMetaData(int index, const vec3& pattern);
	string name() const override { return "WorldEditTerrainMetaData";	}

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	vec3 pattern;
};



#endif //ACTIONWORLDEDITTERRAINMETADATA_H

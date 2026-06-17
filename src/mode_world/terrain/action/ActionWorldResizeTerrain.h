//
// Created by Michael Ankele on 2025-04-23.
//

#ifndef ACTIONWORLDRESIZETERRAIN_H
#define ACTIONWORLDRESIZETERRAIN_H


#include <lib/history/Action.h>
#include <lib/math/vec3.h>

class ActionWorldResizeTerrain : public history::Action {
public:
	ActionWorldResizeTerrain(int index, int nx, int nz);
	string name() const override { return "WorldResizeTerrain";	}

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	int nx, nz;
	Array<float> heights;
};



#endif //ACTIONWORLDRESIZETERRAIN_H

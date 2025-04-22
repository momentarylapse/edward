//
// Created by Michael Ankele on 2025-04-23.
//

#ifndef ACTIONWORLDRESIZETERRAIN_H
#define ACTIONWORLDRESIZETERRAIN_H


#include <action/Action.h>
#include <lib/math/vec3.h>

class ActionWorldResizeTerrain : public Action {
public:
	ActionWorldResizeTerrain(int index, int nx, int nz);
	string name() override { return "WorldResizeTerrain";	}

	void* execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	int nx, nz;
	Array<float> heights;
};



#endif //ACTIONWORLDRESIZETERRAIN_H

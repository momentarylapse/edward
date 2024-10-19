/*
 * ActionWorldEditCamera.h
 *
 *  Created on: 9 Sept 2023
 *      Author: michi
 */

#ifndef SRC_ACTION_WORLD_CAMERA_ACTIONWORLDEDITCAMERA_H_
#define SRC_ACTION_WORLD_CAMERA_ACTIONWORLDEDITCAMERA_H_

#include "../../Action.h"
#include "../../../data/world/WorldCamera.h"

class ActionWorldEditCamera : public Action {
public:
	ActionWorldEditCamera(int index, const WorldCamera &c);
	string name() override { return "WorldEditCamera"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	WorldCamera camera;
};

#endif /* SRC_ACTION_WORLD_CAMERA_ACTIONWORLDEDITCAMERA_H_ */

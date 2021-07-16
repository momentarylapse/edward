/*
 * WorldCamera.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDCAMERA_H_
#define SRC_DATA_WORLD_WORLDCAMERA_H_

#include "../../MultiView/SingleData.h"

class ScriptInstanceData;


class WorldCamera: public MultiView::SingleData {
public:
	string name;
	vector ang = vector::ZERO;
	float fov = pi/4;
	float min_depth = 1;
	float max_depth = 10000;
	float exposure = 1;
	Array<ScriptInstanceData> components;
};


#endif /* SRC_DATA_WORLD_WORLDCAMERA_H_ */

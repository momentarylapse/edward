/*
 * WorldCamera.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDCAMERA_H_
#define SRC_DATA_WORLD_WORLDCAMERA_H_

#include "../../MultiView/SingleData.h"


class WorldCamera: public MultiView::SingleData {
public:
	WorldCamera();
	string name;
	vector ang;
	float fov;
	float min_depth, max_depth;
	float exposure;
};


#endif /* SRC_DATA_WORLD_WORLDCAMERA_H_ */

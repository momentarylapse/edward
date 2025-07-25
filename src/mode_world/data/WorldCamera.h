/*
 * WorldCamera.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDCAMERA_H_
#define SRC_DATA_WORLD_WORLDCAMERA_H_

#include <view/SingleData.h>


struct WorldCamera {
	string name;
	float fov = pi/4;
	float min_depth = 1;
	float max_depth = 10000;
	float exposure = 1;
	float bloom_factor = 0.15f;
};


#endif /* SRC_DATA_WORLD_WORLDCAMERA_H_ */

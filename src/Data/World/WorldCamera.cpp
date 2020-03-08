/*
 * WorldCamera.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldCamera.h"



WorldCamera::WorldCamera() {
	fov = pi/4;
	min_depth = 1;
	max_depth = 10000;
	exposure = 1;
}


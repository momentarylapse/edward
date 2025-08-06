/*
 * WorldLight.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDLIGHT_H_
#define SRC_DATA_WORLD_WORLDLIGHT_H_

#include "../../lib/image/color.h"
#include <y/world/Light.h>



string light_type(yrenderer::LightType t);
string light_type_canonical(yrenderer::LightType t);

struct WorldLight {
	string name;
	yrenderer::LightType type = yrenderer::LightType::POINT;
	float radius = 100;
	float theta = 0;
	bool enabled = true;
	color col = White;
	float harshness = 0.7f;

	color ambient();
	color diffuse();
};



#endif /* SRC_DATA_WORLD_WORLDLIGHT_H_ */

/*
 * WorldLight.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDLIGHT_H_
#define SRC_DATA_WORLD_WORLDLIGHT_H_

#include "../../multiview/SingleData.h"
#include "../../lib/image/color.h"
#include <y/world/Light.h>


class ScriptInstanceData;


string light_type(LightType t);
string light_type_canonical(LightType t);

class WorldLight: public multiview::SingleData {
public:
	string name;
	LightType type = LightType::POINT;
	vec3 ang = vec3::ZERO;
	float radius = 100;
	float theta = 0;
	bool enabled = true;
	color col = White;
	float harshness = 0.7f;

	Array<ScriptInstanceData> components;

	color ambient();
	color diffuse();
};



#endif /* SRC_DATA_WORLD_WORLDLIGHT_H_ */

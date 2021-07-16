/*
 * WorldLight.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDLIGHT_H_
#define SRC_DATA_WORLD_WORLDLIGHT_H_

#include "../../MultiView/SingleData.h"
#include "../../lib/image/color.h"


class ScriptInstanceData;

enum class LightType {
	DIRECTIONAL,
	POINT,
	CONE
};

string light_type(LightType t);
string light_type_canonical(LightType t);

class WorldLight: public MultiView::SingleData {
public:
	string name;
	LightType type = LightType::POINT;
	vector ang = vector::ZERO;
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

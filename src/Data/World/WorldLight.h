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


enum class LightType {
	DIRECTIONAL,
	POINT,
	CONE
};

string light_type(LightType t);
string light_type_canonical(LightType t);

class WorldLight: public MultiView::SingleData {
public:
	WorldLight();
	string name;
	LightType type;
	vector ang;
	float radius, theta;
	bool enabled;
	color col;
	float harshness;

	color ambient();
	color diffuse();
};



#endif /* SRC_DATA_WORLD_WORLDLIGHT_H_ */

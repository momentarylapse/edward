/*
 * WorldLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldLight.h"


string light_type_canonical(LightType t) {
	if (t == LightType::DIRECTIONAL)
		return "directional";
	if (t == LightType::POINT)
		return "point";
	if (t == LightType::CONE)
		return "cone";
	return "???";
}

string light_type(LightType t) {
	if (t == LightType::DIRECTIONAL)
		return "directional";
	if (t == LightType::POINT)
		return "point";
	if (t == LightType::CONE)
		return "cone";
	return "???";
}

WorldLight::WorldLight() {
	theta = 0;
	radius = 0;
	ang = v_0;
	type = LightType::POINT;
	col = White;
	enabled = true;
	harshness = 0.7f;
}

color WorldLight::ambient() {
	return col * ((1 - harshness) / 2);
}

color WorldLight::diffuse() {
	return col * harshness;
}


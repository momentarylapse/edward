/*
 * WorldLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldLight.h"


string light_type_canonical(yrenderer::LightType t) {
	if (t == yrenderer::LightType::DIRECTIONAL)
		return "directional";
	if (t == yrenderer::LightType::POINT)
		return "point";
	if (t == yrenderer::LightType::CONE)
		return "cone";
	return "???";
}

string light_type(yrenderer::LightType t) {
	if (t == yrenderer::LightType::DIRECTIONAL)
		return "directional";
	if (t == yrenderer::LightType::POINT)
		return "point";
	if (t == yrenderer::LightType::CONE)
		return "cone";
	return "???";
}


color WorldLight::ambient() {
	return col * ((1 - harshness) / 2);
}

color WorldLight::diffuse() {
	return col * harshness;
}


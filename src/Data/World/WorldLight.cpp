/*
 * WorldLight.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldLight.h"



color WorldLight::ambient() {
	return col * ((1 - harshness) / 2);
}

color WorldLight::diffuse() {
	return col * harshness;
}


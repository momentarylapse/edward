/*
 * WorldObject.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldObject.h"
#include "DataWorld.h"

WorldObject::~WorldObject() = default;


void WorldObject::update_data() {
	if (!object)
		return;
	/*object->pos = pos;
	object->ang = quaternion::rotation_v(ang);
	object->update_matrix();*/
}

float WorldObject::hover_distance(multiview::Window* win, const vec2& m, vec3& tp, float& z) {
	return 0;
}

bool WorldObject::overlap_rect(multiview::Window* win, const rect& r) {
	return false;
}

bool WorldObject::in_rect(multiview::Window* win, const rect& r) {
	return false;
}






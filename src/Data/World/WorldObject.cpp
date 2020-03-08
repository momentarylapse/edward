/*
 * WorldObject.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldObject.h"
#include "../../x/object.h"

void WorldObject::update_data() {
	if (!object)
		return;
	object->pos = pos;
	object->ang = quaternion::rotation_v(ang);
	object->update_matrix();
}



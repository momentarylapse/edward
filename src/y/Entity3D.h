/*
 * Entity3D.h
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#pragma once

#include "../y/Entity.h"
#include "../lib/math/vector.h"
#include "../lib/math/quaternion.h"

class matrix;


class Entity3D : public Entity {
public:
	Entity3D();
	Entity3D(const vector &pos, const quaternion &ang);

	vector pos;
	quaternion ang;
	matrix get_local_matrix() const;
	matrix get_matrix() const;

	int object_id;
	Entity3D *parent;
	Entity3D *_cdecl root() const;
};


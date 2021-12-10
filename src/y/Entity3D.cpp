/*
 * Entity3D.cpp
 *
 *  Created on: Jul 15, 2021
 *      Author: michi
 */

#include "Entity3D.h"
#include "../lib/math/matrix.h"


Entity3D::Entity3D() : Entity3D(vector::ZERO, quaternion::ID) {}

Entity3D::Entity3D(const vector &_pos, const quaternion &_ang) : Entity(Entity::Type::ENTITY3D) {
	pos = _pos;
	ang = _ang;
	parent = nullptr;
	object_id = -1;
}

matrix Entity3D::get_local_matrix() const {
	return matrix::translation(pos) * matrix::rotation(ang);
}

matrix Entity3D::get_matrix() const {
	if (parent)
		return parent->get_matrix() * get_local_matrix();
	return get_local_matrix();
}


Entity3D *Entity3D::root() const {
	Entity3D *next = const_cast<Entity3D*>(this);
	while (next->parent)
		next = next->parent;
	return next;
}

